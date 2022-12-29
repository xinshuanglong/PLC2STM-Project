#ifndef _IIC3_H
#define _IIC3_H
#include "sys.h"

#define IIC3_GPIO_PORT GPIOB
#define IIC3_SCL_GPIO_PIN GPIO_Pin_7
#define IIC3_SDA_GPIO_PIN GPIO_Pin_6

// IO��������
#define IIC3_SCL PBout(7)     // SCL
#define IIC3_SDA PBout(6)     // SDA
#define IIC3_READ_SDA PBin(6) // ����SDA

void IIC3_SDA_IN(void);
void IIC3_SDA_OUT(void);

// IIC3���в�������
void IIC3_Init(void);                 // ��ʼ��IIC3��IO��
void IIC3_Start(void);                // ����IIC3��ʼ�ź�
void IIC3_Stop(void);                 // ����IIC3ֹͣ�ź�
void IIC3_Send_Byte(u8 txd);          // IIC3����һ���ֽ�
u8 IIC3_Read_Byte(unsigned char ack); // IIC3��ȡһ���ֽ�
u8 IIC3_Wait_Ack(void);               // IIC3�ȴ�ACK�ź�
void IIC3_Send_Ack(void);             // IIC3����ACK�ź�
void IIC3_Send_NAck(void);            // IIC3������ACK�ź�

#endif
