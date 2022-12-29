#ifndef _IIC2_H
#define _IIC2_H
#include "sys.h"

#define IIC2_GPIO_PORT GPIOF
#define IIC2_SCL_GPIO_PIN GPIO_Pin_7
#define IIC2_SDA_GPIO_PIN GPIO_Pin_8

//IO操作函数
#define IIC2_SCL PFout(7)     //SCL
#define IIC2_SDA PFout(8)     //SDA
#define IIC2_READ_SDA PFin(8) //输入SDA

void IIC2_SDA_IN(void);
void IIC2_SDA_OUT(void);

//IIC2所有操作函数
void IIC2_Init(void);                 //初始化IIC2的IO口
void IIC2_Start(void);                //发送IIC2开始信号
void IIC2_Stop(void);                 //发送IIC2停止信号
void IIC2_Send_Byte(u8 txd);          //IIC2发送一个字节
u8 IIC2_Read_Byte(unsigned char ack); //IIC2读取一个字节
u8 IIC2_Wait_Ack(void);               //IIC2等待ACK信号
void IIC2_Send_Ack(void);             //IIC2发送ACK信号
void IIC2_Send_NAck(void);            //IIC2不发送ACK信号

#endif
