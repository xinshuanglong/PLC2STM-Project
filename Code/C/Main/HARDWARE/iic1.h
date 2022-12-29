#ifndef _IIC1_H
#define _IIC1_H

#include "sys.h"

#define IIC1_GPIO_PORT GPIOG
#define IIC1_SCL_GPIO_PIN GPIO_Pin_12
#define IIC1_SDA_GPIO_PIN GPIO_Pin_13

//IO操作函数
#define IIC1_SCL PGout(12)     //SCL
#define IIC1_SDA PGout(13)     //SDA
#define IIC1_READ_SDA PGin(13) //输入SDA

void IIC1_SDA_IN(void);
void IIC1_SDA_OUT(void);

//IIC1所有操作函数
void IIC1_Init(void);        //初始化IIC1的IO口
void IIC1_Start(void);       //发送IIC1开始信号
void IIC1_Stop(void);        //发送IIC1停止信号
void IIC1_Send_Byte(u8 txd); //IIC1发送一个字节
u8 IIC1_Read_Byte(unsigned char ack);
u8 IIC1_Wait_Ack(void);    //IIC1等待ACK信号
void IIC1_Send_Ack(void);  //IIC1发送ACK信号
void IIC1_Send_NAck(void); //IIC1不发送ACK信号

#endif
