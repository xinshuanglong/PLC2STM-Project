#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

typedef struct
{
    GPIO_TypeDef *GPIOx;        // GPIO��
    uint16_t GPIO_Pin;          // GPIO���еĹܽ�
    GPIOMode_TypeDef GPIO_Mode; // GPIO������ģʽ
    bool reverse;               // IO�Ƿ���(�ߵ�ƽ���ǵ͵�ƽ��Ч �Ƿ���)
} Gpio_Type;

void Gpio_Bit_Init(const Gpio_Type *gpio);

void Gpio_Set_Bit(const Gpio_Type *gpio);
void Gpio_Reset_Bit(const Gpio_Type *gpio);

uint8_t Gpio_Read_Output_Bit(const Gpio_Type *gpio);
uint8_t Gpio_Read_Input_Bit(const Gpio_Type *gpio);

#endif
