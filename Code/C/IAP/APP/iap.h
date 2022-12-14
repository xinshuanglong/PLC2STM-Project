#ifndef __IAP_H__
#define __IAP_H__
#include "sys.h"  
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
typedef  void (*iapfun)(void);				//定义一个函数类型的参数.   

void iap_load_app(u32 appxaddr);			//跳转到APP程序执行
u8 iap_write_a_line(u32 appxaddr,u8 *appbuf);
u8 Flash_Write(u32 appxaddr,u8 *appbuf,u32 len);
u8 Flash_Read(u32 appxaddr,u8 *appbuf,u32 len);

void Boot2App(void);


#endif

