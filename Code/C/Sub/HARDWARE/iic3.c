#include "sys.h"
#include "iic3.h"
#include "delay.h"

void IIC3_SDA_IN(void)
{
    u32 dataTemp = GPIOB->MODER;
    dataTemp &= ~(3 << 12);
    dataTemp |= (u32)0 << 12;
    GPIOB->MODER = dataTemp;
}

void IIC3_SDA_OUT(void)
{
    u32 dataTemp = GPIOB->MODER;
    dataTemp &= ~(3 << 12);
    dataTemp |= (u32)1 << 12;
    GPIOB->MODER = dataTemp;
}

// ��ʼ��IIC3
void IIC3_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = IIC3_SCL_GPIO_PIN | IIC3_SDA_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;      // ��ͨ���ģʽ
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     // �������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // IO���ٶ�Ϊ100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       // ����

    GPIO_Init(IIC3_GPIO_PORT, &GPIO_InitStructure);
    GPIO_SetBits(IIC3_GPIO_PORT, IIC3_SCL_GPIO_PIN | IIC3_SDA_GPIO_PIN);
    IIC3_SDA = 1;
    IIC3_SCL = 1;
}

void IIC3_Start(void)
{
    IIC3_SDA_OUT();
    IIC3_SDA = 1;
    IIC3_SCL = 1;
    delay_us(4);
    IIC3_SDA = 0; // ʱ�Ӹ�ʱ�������ź���
    delay_us(4);
    IIC3_SCL = 0;
}

void IIC3_Stop(void)
{
    IIC3_SDA_OUT();
    IIC3_SDA = 0;
    IIC3_SCL = 0;
    delay_us(4);
    IIC3_SCL = 1;
    delay_us(4);
    IIC3_SDA = 1; // ʱ�Ӹ�ʱ�������ź���
    delay_us(4);
    IIC3_SCL = 1;
}

/*
 *	�ȴ�ACK
 *   ����ֵ1 �ȴ�ʧ��
 *   ����ֵ0 �ȴ�ACK�ɹ�
 */
u8 IIC3_Wait_Ack(void)
{
    u8 ucOvertime = 0;
    IIC3_SDA_IN();
    IIC3_SCL = 0;
    delay_us(4);
    IIC3_SCL = 1;
    delay_us(2);
    while (IIC3_READ_SDA)
    {
        ucOvertime++;
        {
            if (ucOvertime > 250)
            {
                IIC3_Stop();
                return 1;
            }
        }
    }
    delay_us(4);
    IIC3_SCL = 0;
    return 0;
}

void IIC3_Send_Ack(void)
{
    IIC3_SDA_OUT();
    IIC3_SCL = 0;
    delay_us(2);
    IIC3_SDA = 0;
    delay_us(2);
    IIC3_SCL = 1;
    delay_us(4);
    IIC3_SCL = 0;
}

void IIC3_Send_NAck(void)
{
    IIC3_SDA_OUT();
    IIC3_SCL = 0;
    delay_us(2);
    IIC3_SDA = 1;
    delay_us(2);
    IIC3_SCL = 1;
    delay_us(4);
    IIC3_SCL = 0;
}

/* IIC3����һ���ֽ� */
void IIC3_Send_Byte(u8 data)
{
    u8 i;
    IIC3_SDA_OUT();
    IIC3_SCL = 0;
    for (i = 0; i < 8; i++)
    {
        delay_us(2);
        IIC3_SDA = (data & 0x80) >> 7;
        data <<= 1;
        delay_us(2);
        IIC3_SCL = 1;
        delay_us(4);
        IIC3_SCL = 0;
    }
}

// ��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK
u8 IIC3_Read_Byte(unsigned char ack)
{
    u8 i, receive_data = 0;
    IIC3_SDA_IN();
    IIC3_SCL = 0;
    for (i = 0; i < 8; i++)
    {
        delay_us(4);
        IIC3_SCL = 1;
        delay_us(2);
        receive_data <<= 1;
        if (IIC3_READ_SDA)
            receive_data |= 1;
        delay_us(2);
        IIC3_SCL = 0;
    }

    if (!ack)
        IIC3_Send_NAck(); // ����nACK
    else
        IIC3_Send_Ack(); // ����ACK

    return receive_data;
}
