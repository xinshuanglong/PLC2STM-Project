#ifndef _IIC3_H
#define _IIC3_H
#include "sys.h"

#define IIC3_GPIO_PORT GPIOB
#define IIC3_SCL_GPIO_PIN GPIO_Pin_7
#define IIC3_SDA_GPIO_PIN GPIO_Pin_6

// IO操作函数
#define IIC3_SCL PBout(7)     // SCL
#define IIC3_SDA PBout(6)     // SDA
#define IIC3_READ_SDA PBin(6) // 输入SDA

void IIC3_SDA_IN(void);
void IIC3_SDA_OUT(void);

// IIC3所有操作函数
void IIC3_Init(void);                 // 初始化IIC3的IO口
void IIC3_Start(void);                // 发送IIC3开始信号
void IIC3_Stop(void);                 // 发送IIC3停止信号
void IIC3_Send_Byte(u8 txd);          // IIC3发送一个字节
u8 IIC3_Read_Byte(unsigned char ack); // IIC3读取一个字节
u8 IIC3_Wait_Ack(void);               // IIC3等待ACK信号
void IIC3_Send_Ack(void);             // IIC3发送ACK信号
void IIC3_Send_NAck(void);            // IIC3不发送ACK信号

#endif
