#include "sys.h"
#include "bsp_iwdg.h"

#define IWDG_PORT       GPIOA
#define IWDG_PIN        GPIO_Pin_1


//内部看门狗与硬件看门狗
void IWDG_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//使能GPIO时钟


  //IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  //IWDG_SetPrescaler(IWDG_Prescaler_128);
  //IWDG_Enable();
  DBGMCU_Config(DBGMCU_IWDG_STOP, ENABLE);   //调试断点时，停止看门狗计数
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  IWDG_SetPrescaler(IWDG_Prescaler_128);
  IWDG_Enable();


  //硬件看门设置
  GPIO_InitStructure.GPIO_Pin = IWDG_PIN ; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(IWDG_PORT, &GPIO_InitStructure);//初始化GPIO

  GPIO_SetBits(IWDG_PORT,IWDG_PIN);

  IWDG_Feed();
  
}



//由于硬件看门狗需要在1.6秒内复位，所以需要1.6秒之内调用此函数，以免系统复位
void IWDG_Feed(void)
{
  IWDG_ReloadCounter();
  GPIO_ToggleBits(IWDG_PORT,IWDG_PIN);
}


