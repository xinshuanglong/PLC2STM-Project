#include "sys.h"
#include "iic2.h"
#include "delay.h"

void IIC2_SDA_IN(void)
{
    u32 dataTemp = GPIOF->MODER;
    dataTemp &= ~(3 << 16);
    dataTemp |= (u32)0 << 16;
    GPIOF->MODER = dataTemp;
}

void IIC2_SDA_OUT(void)
{
    u32 dataTemp = GPIOF->MODER;
    dataTemp &= ~(3 << 16);
    dataTemp |= (u32)1 << 16;
    GPIOF->MODER = dataTemp;
}

//初始化IIC2
void IIC2_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    GPIO_InitStructure.GPIO_Pin = IIC2_SCL_GPIO_PIN | IIC2_SDA_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;      //普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //IO口速度为100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       //上拉

    GPIO_Init(IIC2_GPIO_PORT, &GPIO_InitStructure);
    GPIO_SetBits(IIC2_GPIO_PORT, IIC2_SCL_GPIO_PIN | IIC2_SDA_GPIO_PIN);
    IIC2_SDA = 1;
    IIC2_SCL = 1;
}

void IIC2_Start(void)
{
    IIC2_SDA_OUT();
    IIC2_SDA = 1;
    IIC2_SCL = 1;
    delay_us(4);
    IIC2_SDA = 0; //时钟高时，拉低信号线
    delay_us(4);
    IIC2_SCL = 0;
}

void IIC2_Stop(void)
{
    IIC2_SDA_OUT();
    IIC2_SDA = 0;
    IIC2_SCL = 0;
    delay_us(4);
    IIC2_SCL = 1;
    delay_us(4);
    IIC2_SDA = 1; //时钟高时，拉高信号线
    delay_us(4);
    IIC2_SCL = 1;
}

/*
*	等待ACK
*   返回值1 等待失败
*   返回值0 等待ACK成功
*/
u8 IIC2_Wait_Ack(void)
{
    u8 ucOvertime = 0;
    IIC2_SDA_IN();
    IIC2_SCL = 0;
    delay_us(4);
    IIC2_SCL = 1;
    delay_us(2);
    while (IIC2_READ_SDA)
    {
        ucOvertime++;
        {
            if (ucOvertime > 250)
            {
                IIC2_Stop();
                return 1;
            }
        }
    }
    delay_us(4);
    IIC2_SCL = 0;
    return 0;
}

void IIC2_Send_Ack(void)
{
    IIC2_SDA_OUT();
    IIC2_SCL = 0;
    delay_us(2);
    IIC2_SDA = 0;
    delay_us(2);
    IIC2_SCL = 1;
    delay_us(4);
    IIC2_SCL = 0;
}

void IIC2_Send_NAck(void)
{
    IIC2_SDA_OUT();
    IIC2_SCL = 0;
    delay_us(2);
    IIC2_SDA = 1;
    delay_us(2);
    IIC2_SCL = 1;
    delay_us(4);
    IIC2_SCL = 0;
}

/* IIC2发送一个字节 */
void IIC2_Send_Byte(u8 data)
{
    u8 i;
    IIC2_SDA_OUT();
    IIC2_SCL = 0;
    for (i = 0; i < 8; i++)
    {
        delay_us(2);
        IIC2_SDA = (data & 0x80) >> 7;
        data <<= 1;
        delay_us(2);
        IIC2_SCL = 1;
        delay_us(4);
        IIC2_SCL = 0;
    }
}

//读1个字节，ack=1时，发送ACK，ack=0，发送nACK
u8 IIC2_Read_Byte(unsigned char ack)
{
    u8 i, receive_data = 0;
    IIC2_SDA_IN();
    IIC2_SCL = 0;
    for (i = 0; i < 8; i++)
    {
        delay_us(4);
        IIC2_SCL = 1;
        delay_us(2);
        receive_data <<= 1;
        if (IIC2_READ_SDA)
            receive_data |= 1;
        delay_us(2);
        IIC2_SCL = 0;
    }

    if (!ack)
        IIC2_Send_NAck(); //发送nACK
    else
        IIC2_Send_Ack(); //发送ACK

    return receive_data;
}
