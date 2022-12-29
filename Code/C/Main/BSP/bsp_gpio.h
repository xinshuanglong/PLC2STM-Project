#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

typedef struct
{
    GPIO_TypeDef *GPIOx;        // GPIO组
    uint16_t GPIO_Pin;          // GPIO组中的管脚
    GPIOMode_TypeDef GPIO_Mode; // GPIO的运行模式
    bool reverse;               // IO是否反向(高电平还是低电平有效 是否反向)
} Gpio_Type;

void Gpio_Bit_Init(const Gpio_Type *gpio);

void Gpio_Set_Bit(const Gpio_Type *gpio);
void Gpio_Reset_Bit(const Gpio_Type *gpio);

uint8_t Gpio_Read_Output_Bit(const Gpio_Type *gpio);
uint8_t Gpio_Read_Input_Bit(const Gpio_Type *gpio);

#endif
