#ifndef _IIC1_H
#define _IIC1_H

#include "sys.h"

#define IIC1_GPIO_PORT GPIOG
#define IIC1_SCL_GPIO_PIN GPIO_Pin_12
#define IIC1_SDA_GPIO_PIN GPIO_Pin_13

//IO��������
#define IIC1_SCL PGout(12)     //SCL
#define IIC1_SDA PGout(13)     //SDA
#define IIC1_READ_SDA PGin(13) //����SDA

void IIC1_SDA_IN(void);
void IIC1_SDA_OUT(void);

//IIC1���в�������
void IIC1_Init(void);        //��ʼ��IIC1��IO��
void IIC1_Start(void);       //����IIC1��ʼ�ź�
void IIC1_Stop(void);        //����IIC1ֹͣ�ź�
void IIC1_Send_Byte(u8 txd); //IIC1����һ���ֽ�
u8 IIC1_Read_Byte(unsigned char ack);
u8 IIC1_Wait_Ack(void);    //IIC1�ȴ�ACK�ź�
void IIC1_Send_Ack(void);  //IIC1����ACK�ź�
void IIC1_Send_NAck(void); //IIC1������ACK�ź�

#endif
