#include "sys.h"
#include "delay.h"
#include "stmflash.h"
#include "iap.h" 

#include "config.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//IAP ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/7/21
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 

iapfun jump2app; 
u32 iapbuf[512];    //2K�ֽڻ���  

u32 readBuf[1024];  //4K�ֽڻ���


//appxaddr:Ӧ�ó������ʼ��ַ ��ؽ���32Bit����
//appbuf:Ӧ�ó���CODE
//len:д���С
//����ֵ1 �ɹ�
//����ֵ0 ʧ��(��ַ����)
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
    dfu+=4;//ƫ��4���ֽ�
    iapbuf[i++]=temp;
  }   

  //���⴦��
  if(appxaddr>=APP_CONFIG_FLASH_ADDR_START && appxaddr<APP1_FLASH_ADDR_START){
    STMFLASH_Read(APP_CONFIG_FLASH_ADDR_START,readBuf,1024);
    memcpy((u8 *)readBuf+appxaddr-APP_CONFIG_FLASH_ADDR_START,(u8 *)iapbuf,i*4);
    STMFLASH_Write(APP_CONFIG_FLASH_ADDR_START,readBuf,1024);
  }else{
    STMFLASH_Write(appxaddr,iapbuf,i);
  }

  return 1;  
}


//appxaddr:Ӧ�ó������ʼ��ַ
//appbuf:Ӧ�ó���CODE
//len:д���С
//����ֵ1 �ɹ�
//����ֵ0 ʧ��(��ַ����)
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
    dfu+=4;//ƫ��4���ֽ�
    i++;
  }

  return 1;  
}



//��ת��Ӧ�ó����
//appxaddr:�û�������ʼ��ַ.
void iap_load_app(u32 appxaddr)
{
  if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000)   //���ջ����ַ�Ƿ�Ϸ�.
  {   
    jump2app=(iapfun)*(vu32*)(appxaddr+4);          //�û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)		
    MSR_MSP(*(vu32*)appxaddr);                      //��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)
    jump2app();                                     //��ת��APP.
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
    if(((*(vu32*)(APP1_FLASH_ADDR_START+4))&0xFF000000)==0x08000000)//�ж��Ƿ�Ϊ0X08XXXXXX.
      iap_load_app(APP1_FLASH_ADDR_START);//ִ��FLASH APP���� 
  } 
} 

