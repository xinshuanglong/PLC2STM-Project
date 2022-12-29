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
        // printf("SD����ʼ��[ʧ��]...\r\n");
    }

    if (f_mount(&fs[0], "", 1) == 0)
    {
        SDSetST(1);
        printf("SD������[�ɹ�]...\r\n");
    }
    else
    {
        SDSetST(0);
        // printf("SD������[ʧ��]...\r\n");
    }
}

/***********************************************/
// �����ջ��С
#define SD_TASK_STK_SIZE 256
// ������
TaskHandle_t SDTaskHandler;
// ������
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
            // printf("δ��⵽SD��...\r\n");
            SDSetST(0);
            SDCardMount();
        }

        vTaskDelay(5000);
    }
}
