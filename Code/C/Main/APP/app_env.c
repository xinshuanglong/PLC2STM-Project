#include "bsp_include.h"

_Time gtTimeBdc, gtTimeBin;

/***********************************************/
// 任务堆栈大小
#define ENV_TASK_STK_SIZE 128
// 任务句柄
TaskHandle_t EnvTaskHandler;
// 任务函数
void EnvTask(void *pvParameters);
/***********************************************/

void EnvTaskCreat(void)
{
    xTaskCreate(EnvTask,
                "EnvTask",
                ENV_TASK_STK_SIZE,
                NULL,
                ENV_TASK_PRIO,
                EnvTaskHandler);
}

void EnvTask(void *pvParameters)
{
    uint32_t count = 0;

    memset(&gtTimeBdc, 0, sizeof(gtTimeBdc));

    vTaskDelay(1000); // 等待ADC开始并稳定

    while (1)
    {
        // 时间
        if (RTC_Get_Time(&gtTimeBdc, &gtTimeBin))
        {
        }

        delay_ms(500);

        count++;
    }
}
