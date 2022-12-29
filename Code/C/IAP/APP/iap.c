#include "sys.h"
#include "delay.h"
#include "stmflash.h"
#include "iap.h" 

#include "config.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//IAP 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/7/21
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 

iapfun jump2app; 
u32 iapbuf[512];    //2K字节缓存  

u32 readBuf[1024];  //4K字节缓存


//appxaddr:应用程序的起始地址 务必进行32Bit对齐
//appbuf:应用程序CODE
//len:写入大小
//返回值1 成功
//返回值0 失败(地址超限)
u8 Flash_Write(u32 appxaddr,u8 *appbuf,u32 len)
{
  u32 t;
  u16 i=0;
  u32 temp;
  u8 *dfu=appbuf;

  if(appxaddr%4)                return 0;

  for(t=0;t<len;t+=4)
  { 
    temp=(u32)dfu[3]<<24;   
    temp|=(u32)dfu[2]<<16;    
    temp|=(u32)dfu[1]<<8;
    temp|=(u32)dfu[0];
    dfu+=4;//偏移4个字节
    iapbuf[i++]=temp;
  }   

  //特殊处理
  if(appxaddr>=APP_CONFIG_FLASH_ADDR_START && appxaddr<APP1_FLASH_ADDR_START){
    STMFLASH_Read(APP_CONFIG_FLASH_ADDR_START,readBuf,1024);
    memcpy((u8 *)readBuf+appxaddr-APP_CONFIG_FLASH_ADDR_START,(u8 *)iapbuf,i*4);
    STMFLASH_Write(APP_CONFIG_FLASH_ADDR_START,readBuf,1024);
  }else{
    STMFLASH_Write(appxaddr,iapbuf,i);
  }

  return 1;  
}


//appxaddr:应用程序的起始地址
//appbuf:应用程序CODE
//len:写入大小
//返回值1 成功
//返回值0 失败(地址超限)
u8 Flash_Read(u32 appxaddr,u8 *appbuf,u32 len)
{
  u32 t;
  u16 i=0;
  u8 *dfu=appbuf;

  memset(iapbuf,0,sizeof(iapbuf)/sizeof(iapbuf[0]));
  STMFLASH_Read(appxaddr,iapbuf,len/4);


  for(t=0;t<len;t+=4)
  { 
    dfu[0] = (u8)iapbuf[i];
    dfu[1] = (u8)(iapbuf[i]>>8);
    dfu[2] = (u8)(iapbuf[i]>>16);
    dfu[3] = (u8)(iapbuf[i]>>24);
    dfu+=4;//偏移4个字节
    i++;
  }

  return 1;  
}



//跳转到应用程序段
//appxaddr:用户代码起始地址.
void iap_load_app(u32 appxaddr)
{
  if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000)   //检查栈顶地址是否合法.
  {   
    jump2app=(iapfun)*(vu32*)(appxaddr+4);          //用户代码区第二个字为程序开始地址(复位地址)		
    MSR_MSP(*(vu32*)appxaddr);                      //初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
    jump2app();                                     //跳转到APP.
  }
}



void Boot2App(void)
{
  u32 length,crc,crcSave=0;
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);

  crcSave = *(u32*)(FLASH_SOFTWARE_CRC_ADDR);
  length  = *(u32*)(FLASH_SOFTWARE_LENGTH_ADDR);

  if(crcSave==0 || length==0 || length==0xFFFFFFFF) return;

  CRC_ResetDR();
  crc = CRC_CalcBlockCRC((u32*)APP1_FLASH_ADDR_START,(length-4)/4);
  
  if(crc==crcSave)
  {
    if(((*(vu32*)(APP1_FLASH_ADDR_START+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
      iap_load_app(APP1_FLASH_ADDR_START);//执行FLASH APP代码 
  } 
} 

