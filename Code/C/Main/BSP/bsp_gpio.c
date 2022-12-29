#include "bsp_include.h"

void Gpio_Bit_Init(const Gpio_Type *gpio)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    uint32_t RCC_AHB1Periph = 1 << ((gpio->GPIOx - GPIOA) / (GPIOB - GPIOA));

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph, ENABLE); //使能GPIO时钟

    GPIO_InitStructure.GPIO_Pin = gpio->GPIO_Pin;
    GPIO_InitStructure.GPIO_Mode = gpio->GPIO_Mode;

    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     //推挽
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       //上拉
    GPIO_Init(gpio->GPIOx, &GPIO_InitStructure);       //初始化GPIO
}

void Gpio_Set_Bit(const Gpio_Type *gpio)
{
    GPIO_SetBits(gpio->GPIOx, gpio->GPIO_Pin);
}

void Gpio_Reset_Bit(const Gpio_Type *gpio)
{
    GPIO_ResetBits(gpio->GPIOx, gpio->GPIO_Pin);
}

uint8_t Gpio_Read_Output_Bit(const Gpio_Type *gpio)
{
    return GPIO_ReadOutputDataBit(gpio->GPIOx, gpio->GPIO_Pin);
}

uint8_t Gpio_Read_Input_Bit(const Gpio_Type *gpio)
{
    return GPIO_ReadInputDataBit(gpio->GPIOx, gpio->GPIO_Pin);
}
