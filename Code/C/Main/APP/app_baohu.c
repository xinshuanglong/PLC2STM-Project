#include "bsp_include.h"

EventGroupHandle_t xBaohuEventGroup;

int Baohu_Start(void)
{
    xEventGroupClearBits(xBaohuEventGroup, STOP_BAOHU_EV_BIT);
    xEventGroupSetBits(xBaohuEventGroup, START_BAOHU_EV_BIT);
    return 0;
}

int Baohu_Stop(void)
{
    xEventGroupSetBits(xBaohuEventGroup, STOP_BAOHU_EV_BIT);
    xEventGroupClearBits(xBaohuEventGroup, START_BAOHU_EV_BIT);
    return 0;
}

/***********************************************/
// �����ջ��С
#define BAOHU_TASK_STK_SIZE 512
// ������
TaskHandle_t BaohuTaskHandler;
// ������
void BaohuTask(void *pvParameters);
/***********************************************/

void BaohuTaskCreat(void)
{
    xBaohuEventGroup = xEventGroupCreate();

    xTaskCreate(BaohuTask,
                "BaohuTask",
                BAOHU_TASK_STK_SIZE,
                NULL,
                BAOHU_TASK_PRIO,
                BaohuTaskHandler);
}

void BaohuTask(void *pvParameters)
{
    while (1)
    {
        EventBits_t ev = xEventGroupGetBits(xBaohuEventGroup);
        if (ev & START_BAOHU_EV_BIT)
        {
            Baohu();
        }
        vTaskDelay(100);
    }
}
