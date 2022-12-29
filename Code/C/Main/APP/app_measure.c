#include "bsp_include.h"

volatile uint8_t gWorkStatus = 0; // 0 空闲
u32 startTime = 0;

_tPara gtPara;

QueueHandle_t MeasureQueue = NULL; // 开始测量信号量
EventGroupHandle_t xMeasureEventGroup;

/***********************************************/
// 任务堆栈大小
#define MEASURE_TASK_STK_SIZE 4096
// 任务句柄
TaskHandle_t MeasureTaskHandler;
// 任务函数
void MeasureTask(void *pvParameters);
/***********************************************/

void MeasureTaskCreat(void)
{
    MeasureQueue = xQueueCreate(1, sizeof(_Measure));
    xMeasureEventGroup = xEventGroupCreate();

    xTaskCreate(MeasureTask,
                "MeasureTask",
                MEASURE_TASK_STK_SIZE,
                NULL,
                MEASURE_TASK_PRIO,
                MeasureTaskHandler);
}

void Measure_Over(void)
{
    LogDatabase_Insert("结束");
    Port_WriteAll(0);
}

void Measure_Stop(int opt)
{
    LogDatabase_Insert("停止[%s]", GetStopCmdCHS(opt));
    xEventGroupSetBits(xMeasureEventGroup, STOP_EV_BIT);
}

void Measure_Start(_Measure measure)
{
    if (gWorkStatus == idle)
    {
        xQueueSend(MeasureQueue, &measure, 200);
    }
    else
    {
    }
}

void Measure_Reset(void)
{
    Alarm_Reset();

    memset(&gtPara, 0, sizeof(_tPara));
    sprintf(gtPara.describe, "---");
    sprintf(gtPara.flowDescribe, "---");

    xEventGroupClearBits(xMeasureEventGroup, 0xFFFFFF);
}

int Project_Run(_Measure measure)
{
    int res = 0;

    // 水箱润洗
    if ((res = Flows_ShuiXiangRunXi_Run()) != IS_OVER)
        return res;

    // 一级水箱进样
    if ((res = Flows_YiJiJinYang_Run()) != IS_OVER)
    {
        // 失败进入储水
        Flows_YiJiChuShui_Run();
        return res;
    }

    // 二级水箱进样
    if ((res = Flows_ErJiJinYang_Run()) != IS_OVER)
    {
        // 失败进入储水
        Flows_YiJiChuShui_Run();
        return res;
    }

    if (measure.isOnlyDuoCan)
    {
        // 只测多餐,直接进入反冲洗
        goto FANG_CHONG_XI;
    }

    // 沉淀
    if ((res = Flows_ChenDian_Run()) != IS_OVER)
        return res;

    if (cfgFlow.haveGuoLv != 0)
    {
        // 水箱过滤
        Flows_ShuiYangGuoLv_Run();
    }

    // 配水
    if ((res = Flows_PeiShui_Run(measure.isJiaBiao)) != IS_OVER)
        return res;

FANG_CHONG_XI:

    if ((res = Flows_FanChongXi_Run()) != IS_OVER)
        return res;

    if ((res = Flows_YiJiChuShui_Run()) != IS_OVER)
        return res;

    return 0;
}

void MeasureTask(void *pvParameters)
{
    _Measure Measure;

    Measure_Reset();
    LogDatabase_Insert("***** 重新开机 %s *****", VERSION);

    while (1)
    {
        // Port_Write(32, 0);

        // Port_Write(40, 1);
        // Port_Write(41, 0);
        // vTaskDelay(10000);
        // Port_Write(40, 0);
        // Port_Write(41, 1);
        // vTaskDelay(10000);
        // Port_Write(40, 1);
        // Port_Write(41, 1);
        // vTaskDelay(10000);

        xQueueReset(MeasureQueue);
        gWorkStatus = 0;
        if (xQueueReceive(MeasureQueue, &Measure, portMAX_DELAY) == pdPASS)
        {
            Measure_Reset();

            LogDatabase_Insert("***** 开始.执行流程 *****");
            gtPara.startTime = xTaskGetTickCount(); // 记录开始时间;

            Project_Run(Measure);
            Measure_Over();
        }
    }
}
