#include "bsp_include.h"

EventGroupHandle_t xBushuiEventGroup;

int ShuiXiangBuShui_Start(void)
{
    xEventGroupClearBits(xBushuiEventGroup, STOP_BUSHUI_EV_BIT);
    xEventGroupSetBits(xBushuiEventGroup, START_BUSHUI_EV_BIT);
    return 0;
}

int ShuiXiangBuShui_Stop(void)
{
    xEventGroupSetBits(xBushuiEventGroup, STOP_BUSHUI_EV_BIT);
    xEventGroupClearBits(xBushuiEventGroup, START_BUSHUI_EV_BIT);
    return 0;
}

/***********************************************/
// �����ջ��С
#define BUSHUI_TASK_STK_SIZE 512
// ������
TaskHandle_t BushuiTaskHandler;
// ������
void BushuiTask(void *pvParameters);
/***********************************************/

void BushuiTaskCreat(void)
{
    xBushuiEventGroup = xEventGroupCreate();

    xTaskCreate(BushuiTask,
                "BushuiTask",
                BUSHUI_TASK_STK_SIZE,
                NULL,
                BUSHUI_TASK_PRIO,
                BushuiTaskHandler);
}

void BushuiTask(void *pvParameters)
{
    while (1)
    {
        EventBits_t ev = xEventGroupGetBits(xBushuiEventGroup);
        if (ev & START_BUSHUI_EV_BIT)
        {
            if (ShuiXiangNeedBuShui())
            {
                Flows_ShuiXiangBuShui_Run();
            }
        }
        vTaskDelay(200);
    }
}
