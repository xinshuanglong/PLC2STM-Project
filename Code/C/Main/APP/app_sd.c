#include "bsp_include.h"

volatile uint8_t sdST = 0xFF;

void SDSetST(uint8_t new)
{
    sdST = new;
}

uint8_t SDGetST(void)
{
    return sdST;
}

FATFS fs[1] = {0};
void SDCardMount(void)
{
    if (disk_initialize(0) != 0)
    {
        SDSetST(0);
        // printf("SD卡初始化[失败]...\r\n");
    }

    if (f_mount(&fs[0], "", 1) == 0)
    {
        SDSetST(1);
        printf("SD卡挂载[成功]...\r\n");
    }
    else
    {
        SDSetST(0);
        // printf("SD卡挂载[失败]...\r\n");
    }
}

/***********************************************/
// 任务堆栈大小
#define SD_TASK_STK_SIZE 256
// 任务句柄
TaskHandle_t SDTaskHandler;
// 任务函数
void SDTask(void *pvParameters);
/***********************************************/

void SDTaskCreat(void)
{
    xTaskCreate(SDTask,
                "SDTask",
                SD_TASK_STK_SIZE,
                NULL,
                SD_TASK_PRIO,
                SDTaskHandler);
}

void SDTask(void *pvParameters)
{
    while (1)
    {
        if (disk_status_my(0) != RES_OK || SDGetST() != 1)
        {
            // printf("未检测到SD卡...\r\n");
            SDSetST(0);
            SDCardMount();
        }

        vTaskDelay(5000);
    }
}
