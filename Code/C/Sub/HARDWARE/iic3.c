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

// 初始化IIC3
void IIC3_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = IIC3_SCL_GPIO_PIN | IIC3_SDA_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;      // 普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // IO口速度为100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       // 上拉

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
    IIC3_SDA = 0; // 时钟高时，拉低信号线
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
    IIC3_SDA = 1; // 时钟高时，拉高信号线
    delay_us(4);
    IIC3_SCL = 1;
}

/*
 *	等待ACK
 *   返回值1 等待失败
 *   返回值0 等待ACK成功
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

/* IIC3发送一个字节 */
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

// 读1个字节，ack=1时，发送ACK，ack=0，发送nACK
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
        IIC3_Send_NAck(); // 发送nACK
    else
        IIC3_Send_Ack(); // 发送ACK

    return receive_data;
}
