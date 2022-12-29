#include "bsp_include.h"

#define IWDG_PORT GPIOC
#define IWDG_PIN GPIO_Pin_5

void IWDG_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); // 使能GPIO时钟

    DBGMCU_Config(DBGMCU_IWDG_STOP, ENABLE); // 调试断点时，停止看门狗计数
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_128); // 40Khz
    IWDG_SetReload(3125);                  // 10秒看门狗时间
    IWDG_Enable();

    // 硬件看门设置
    GPIO_InitStructure.GPIO_Pin = IWDG_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;      // 普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       // 上拉
    GPIO_Init(IWDG_PORT, &GPIO_InitStructure);         // 初始化GPIO

    GPIO_SetBits(IWDG_PORT, IWDG_PIN);

    IWDG_Feed();
}

// 由于硬件看门狗需要在1.6秒内复位，所以需要1.6秒之内调用此函数，以免系统复位
void IWDG_Feed(void)
{
    IWDG_ReloadCounter();
    GPIO_ToggleBits(IWDG_PORT, IWDG_PIN);
}

/***********************************************/
// 任务堆栈大小
#define IWDG_TASK_STK_SIZE 128
// 任务句柄
TaskHandle_t IwdgTaskHandler;
TaskHandle_t IwdgCheckTaskHandler;
// 任务函数
void IwdgTask(void *pvParameters);
void IwdgCheckTask(void *pvParameters);
/***********************************************/

void IwdgTaskCreat(void)
{
    xTaskCreate(IwdgTask,
                "IwdgTask",
                IWDG_TASK_STK_SIZE,
                NULL,
                IWDG_TASK_PRIO,
                IwdgTaskHandler);
}

void IwdgTask(void *pvParameters)
{
    while (1)
    {
        IWDG_Feed();
        vTaskDelay(200);
    }
}
