#ifndef __IAP_H__
#define __IAP_H__
#include "sys.h"  
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ���������ɣ��������������κ���;
//ALIENTEK STM32F407������
//IAP ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/7/21
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) �������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////    
typedef  void (*iapfun)(void);				//����һ���������͵Ĳ���.   

void iap_load_app(u32 appxaddr);			//��ת��APP����ִ��
u8 iap_write_a_line(u32 appxaddr,u8 *appbuf);
u8 Flash_Write(u32 appxaddr,u8 *appbuf,u32 len);
u8 Flash_Read(u32 appxaddr,u8 *appbuf,u32 len);

void Boot2App(void);


#endif
