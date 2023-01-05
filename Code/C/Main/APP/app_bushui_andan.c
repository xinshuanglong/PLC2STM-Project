#include "bsp_include.h"

EventGroupHandle_t xBushuiAndanEventGroup;
EventGroupHandle_t xBushuiAndanEventGroup_JianYi;

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

int BuShuiAndan_Start_JianYi(void)
{
    xEventGroupClearBits(xBushuiAndanEventGroup_JianYi, STOP_BUSHUI_ANDAN_EV_BIT);
    xEventGroupSetBits(xBushuiAndanEventGroup_JianYi, START_BUSHUI_ANDAN_EV_BIT);
    return 0;
}

int BuShuiAndan_Stop_JianYi(void)
{
    xEventGroupSetBits(xBushuiAndanEventGroup_JianYi, STOP_BUSHUI_ANDAN_EV_BIT);
    xEventGroupClearBits(xBushuiAndanEventGroup_JianYi, START_BUSHUI_ANDAN_EV_BIT);
    return 0;
}

/***********************************************/
// 任务堆栈大小
#define BUSHUI_ANDAN_TASK_STK_SIZE 512
// 任务句柄
TaskHandle_t BushuiAndanTaskHandler;
TaskHandle_t BushuiAndanTaskHandler_JianYi;
// 任务函数
void BushuiAndanTask(void *pvParameters);
void BushuiAndanTask_JianYi(void *pvParameters);
/***********************************************/

void BushuiAndanTaskCreat(void)
{
    xBushuiAndanEventGroup = xEventGroupCreate();
    xBushuiAndanEventGroup_JianYi = xEventGroupCreate();

    xTaskCreate(BushuiAndanTask,
                "BushuiAndanTask",
                BUSHUI_ANDAN_TASK_STK_SIZE,
                NULL,
                BUSHUI_ANDAN_TASK_PRIO,
                BushuiAndanTaskHandler);

    xTaskCreate(BushuiAndanTask_JianYi,
                "BushuiAndanTask_JianYi",
                BUSHUI_ANDAN_TASK_STK_SIZE,
                NULL,
                BUSHUI_ANDAN_TASK_PRIO,
                BushuiAndanTaskHandler_JianYi);
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

void BushuiAndanTask_JianYi(void *pvParameters)
{
    while (1)
    {
        EventBits_t ev = xEventGroupGetBits(xBushuiAndanEventGroup_JianYi);
        if (ev & START_BUSHUI_ANDAN_EV_BIT)
        {
            if (AndanNeedBuShui())
            {
                Flows_AndanBuShui_Run_JianYi();
            }
        }
        vTaskDelay(200);
    }
}
