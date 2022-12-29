#include "bsp_include.h"

EventGroupHandle_t xBushuiAndanEventGroup;

int BuShuiAndan_Start(void)
{
    xEventGroupClearBits(xBushuiAndanEventGroup, STOP_BUSHUI_ANDAN_EV_BIT);
    xEventGroupSetBits(xBushuiAndanEventGroup, START_BUSHUI_ANDAN_EV_BIT);
    return 0;
}

int BuShuiAndan_Stop(void)
{
    xEventGroupSetBits(xBushuiAndanEventGroup, STOP_BUSHUI_ANDAN_EV_BIT);
    xEventGroupClearBits(xBushuiAndanEventGroup, START_BUSHUI_ANDAN_EV_BIT);
    return 0;
}

/***********************************************/
// �����ջ��С
#define BUSHUI_ANDAN_TASK_STK_SIZE 512
// ������
TaskHandle_t BushuiAndanTaskHandler;
// ������
void BushuiAndanTask(void *pvParameters);
/***********************************************/

void BushuiAndanTaskCreat(void)
{
    xBushuiAndanEventGroup = xEventGroupCreate();

    xTaskCreate(BushuiAndanTask,
                "BushuiAndanTask",
                BUSHUI_ANDAN_TASK_STK_SIZE,
                NULL,
                BUSHUI_ANDAN_TASK_PRIO,
                BushuiAndanTaskHandler);
}

void BushuiAndanTask(void *pvParameters)
{
    while (1)
    {
        EventBits_t ev = xEventGroupGetBits(xBushuiAndanEventGroup);
        if (ev & START_BUSHUI_ANDAN_EV_BIT)
        {
            if (AndanNeedBuShui())
            {
                Flows_AndanBuShui_Run();
            }
        }
        vTaskDelay(200);
    }
}
