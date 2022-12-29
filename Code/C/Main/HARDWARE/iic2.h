#ifndef _IIC2_H
#define _IIC2_H
#include "sys.h"

#define IIC2_GPIO_PORT GPIOF
#define IIC2_SCL_GPIO_PIN GPIO_Pin_7
#define IIC2_SDA_GPIO_PIN GPIO_Pin_8

//IO��������
#define IIC2_SCL PFout(7)     //SCL
#define IIC2_SDA PFout(8)     //SDA
#define IIC2_READ_SDA PFin(8) //����SDA

void IIC2_SDA_IN(void);
void IIC2_SDA_OUT(void);

//IIC2���в�������
void IIC2_Init(void);                 //��ʼ��IIC2��IO��
void IIC2_Start(void);                //����IIC2��ʼ�ź�
void IIC2_Stop(void);                 //����IIC2ֹͣ�ź�
void IIC2_Send_Byte(u8 txd);          //IIC2����һ���ֽ�
u8 IIC2_Read_Byte(unsigned char ack); //IIC2��ȡһ���ֽ�
u8 IIC2_Wait_Ack(void);               //IIC2�ȴ�ACK�ź�
void IIC2_Send_Ack(void);             //IIC2����ACK�ź�
void IIC2_Send_NAck(void);            //IIC2������ACK�ź�

#endif
