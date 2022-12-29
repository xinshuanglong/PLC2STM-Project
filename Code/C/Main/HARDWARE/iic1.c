#include "sys.h"
#include "iic1.h"
#include "delay.h"

void IIC1_SDA_IN(void)
{
    u32 dataTemp = GPIOG->MODER;
    dataTemp &= ~(3 << 26);
    dataTemp |= (u32)0 << 26;
    GPIOG->MODER = dataTemp;
}

void IIC1_SDA_OUT(void)
{
    u32 dataTemp = GPIOG->MODER;
    dataTemp &= ~(3 << 26);
    dataTemp |= (u32)1 << 26;
    GPIOG->MODER = dataTemp;
}

//��ʼ��IIC1
void IIC1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

    GPIO_InitStructure.GPIO_Pin = IIC1_SCL_GPIO_PIN | IIC1_SDA_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;      //��ͨ���ģʽ
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //IO���ٶ�Ϊ100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       //����

    GPIO_Init(IIC1_GPIO_PORT, &GPIO_InitStructure);
    GPIO_SetBits(IIC1_GPIO_PORT, IIC1_SCL_GPIO_PIN | IIC1_SDA_GPIO_PIN);

    IIC1_SDA = 1;
    IIC1_SCL = 1;
}

void IIC1_Start(void)
{
    IIC1_SDA_OUT();
    IIC1_SDA = 1;
    IIC1_SCL = 1;
    delay_us(4);
    IIC1_SDA = 0; //ʱ�Ӹ�ʱ�������ź���
    delay_us(4);
    IIC1_SCL = 0;
}

void IIC1_Stop(void)
{
    IIC1_SDA_OUT();
    IIC1_SDA = 0;
    IIC1_SCL = 0;
    delay_us(4);
    IIC1_SCL = 1;
    delay_us(4);
    IIC1_SDA = 1; //ʱ�Ӹ�ʱ�������ź���
    delay_us(4);
    IIC1_SCL = 1;
}

/*
*	�ȴ�ACK
*   ����ֵ1 �ȴ�ʧ��
*   ����ֵ0 �ȴ�ACK�ɹ�
*/
u8 IIC1_Wait_Ack(void)
{
    u8 ucOvertime = 0;
    IIC1_SDA_IN();
    IIC1_SCL = 0;
    delay_us(4);
    IIC1_SCL = 1;
    delay_us(2);
    while (IIC1_READ_SDA)
    {
        ucOvertime++;
        {
            if (ucOvertime > 250)
            {
                IIC1_Stop();
                return 1;
            }
        }
    }
    delay_us(4);
    IIC1_SCL = 0;
    return 0;
}

void IIC1_Send_Ack(void)
{
    IIC1_SDA_OUT();
    IIC1_SCL = 0;
    delay_us(2);
    IIC1_SDA = 0;
    delay_us(2);
    IIC1_SCL = 1;
    delay_us(4);
    IIC1_SCL = 0;
}

void IIC1_Send_NAck(void)
{
    IIC1_SDA_OUT();
    IIC1_SCL = 0;
    delay_us(2);
    IIC1_SDA = 1;
    delay_us(2);
    IIC1_SCL = 1;
    delay_us(4);
    IIC1_SCL = 0;
}

/* IIC1����һ���ֽ� */
void IIC1_Send_Byte(u8 data)
{
    u8 i;
    IIC1_SDA_OUT();
    IIC1_SCL = 0;
    for (i = 0; i < 8; i++)
    {
        delay_us(2);
        IIC1_SDA = (data & 0x80) >> 7;
        data <<= 1;
        delay_us(2);
        IIC1_SCL = 1;
        delay_us(4);
        IIC1_SCL = 0;
    }
}

//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK
u8 IIC1_Read_Byte(unsigned char ack)
{
    u8 i, receive_data = 0;
    IIC1_SDA_IN();
    IIC1_SCL = 0;
    for (i = 0; i < 8; i++)
    {
        delay_us(4);
        IIC1_SCL = 1;
        delay_us(2);
        receive_data <<= 1;
        if (IIC1_READ_SDA)
            receive_data |= 1;
        delay_us(2);
        IIC1_SCL = 0;
    }

    if (!ack)
        IIC1_Send_NAck(); //����nACK
    else
        IIC1_Send_Ack(); //����ACK

    return receive_data;
}
