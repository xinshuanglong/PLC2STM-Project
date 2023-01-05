#include "bsp_include.h"

volatile uint8_t gWorkStatus = 0; // 0 ����
u32 startTime = 0;

_tPara gtPara;

QueueHandle_t MeasureQueue = NULL; // ��ʼ�����ź���
EventGroupHandle_t xMeasureEventGroup;

/***********************************************/
// �����ջ��С
#define MEASURE_TASK_STK_SIZE 4096
// ������
TaskHandle_t MeasureTaskHandler;
// ������
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

void Alarm_Save(uint8_t alarm, uint8_t error)
{
    _AlarmDatabase alarmbase;
    memset(&alarmbase, 0, sizeof(alarmbase));

    alarmbase.time = gtTimeBdc;
    alarmbase.id = gtPara.id;
    alarmbase.fid = gtPara.fid;
    alarmbase.step = gtPara.step;
    memcpy(alarmbase.flowDescribe, gtPara.flowDescribe, sizeof(alarmbase.flowDescribe));

    alarmbase.alarm = alarm;
    alarmbase.error = error;

    AlarmDatabase_Insert(alarmbase);
    LogDatabase_Insert("[%s] [%s]", GetAlarmCHS(alarm), GetErrorCHS(error));
}

void Measure_Over(void)
{
    LogDatabase_Insert("����");
}

void Measure_Stop(int opt)
{
    LogDatabase_Insert("ֹͣ[%s]", GetStopCmdCHS(opt));
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

void Test_Time(void)
{
    if (gtPara.time.year < 0x23) // С��2023
    {
        Alarm_Add(0, time_alarm, 0);
    }
}

int Project_Run(_Measure measure)
{
    int res = 0;

    // ��ʼ��
    if ((res = Flows_ChuShiHua_Run()) != IS_OVER)
        return res;

    // ˮ����ϴ
    if ((res = Flows_ShuiXiangRunXi_Run()) != IS_OVER)
        return res;

    // һ��ˮ�����
    if ((res = Flows_YiJiJinYang_Run()) != IS_OVER)
    {
        // ʧ�ܽ��봢ˮ
        Flows_YiJiChuShui_Run();
        return res;
    }

    // ����ˮ�����
    if ((res = Flows_ErJiJinYang_Run()) != IS_OVER)
    {
        // ʧ�ܽ��봢ˮ
        Flows_YiJiChuShui_Run();
        return res;
    }

    // ����
    if ((res = Flows_ChenDian_Run()) != IS_OVER)
        return res;

    if (measure.isOnlyDuoCan)
    {
        // ֻ����,ֱ�ӽ��뷴��ϴ
        goto FANG_CHONG_XI;
    }

    if (cfgFlow.haveGuoLv != 0)
    {
        // ˮ�����
        Flows_ShuiYangGuoLv_Run();
    }

    // ��ˮ
    if ((res = Flows_PeiShui_Run(measure.isJiaBiao)) != IS_OVER)
        return res;

FANG_CHONG_XI:

    if ((res = Flows_FanChongXi_Run()) != IS_OVER)
        return res;

    if ((res = Flows_YiJiChuShui_Run()) != IS_OVER)
        return res;

    return 0;
}

int Project_Run_JianYi(_Measure measure)
{
    int res = 0;

    // ��ʼ��
    if ((res = Flows_ChuShiHua_Run_JianYi()) != IS_OVER)
        return res;

    // ˮ����ϴ
    if ((res = Flows_ShuiXiangRunXi_Run_JianYi()) != IS_OVER)
        return res;

    // ��ͨ�ؽ���
    if ((res = Flows_LiuTongChiJinYang_Run_JianYi()) != IS_OVER)
    {
        // ʧ�ܽ��봢ˮ
        Flows_YiJiChuShui_Run_JianYi();
        return res;
    }

    // ����ؽ���
    if ((res = Flows_ChenDianChiJinYang_Run_JianYi()) != IS_OVER)
    {
        // ʧ�ܽ��봢ˮ
        Flows_YiJiChuShui_Run_JianYi();
        return res;
    }

    // ����
    if ((res = Flows_ChenDian_Run_JianYi()) != IS_OVER)
        return res;

    if (measure.isOnlyDuoCan)
    {
        // ֻ����,ֱ�ӽ��뷴��ϴ
        goto FANG_CHONG_XI_JIANYI;
    }

    if (cfgFlow.haveGuoLv != 0)
    {
        // ˮ�����
        Flows_ShuiYangGuoLv_Run_JianYi();
    }

    // ��ˮ
    if ((res = Flows_PeiShui_Run_JianYi(measure.isJiaBiao)) != IS_OVER)
        return res;

FANG_CHONG_XI_JIANYI:

    if ((res = Flows_FanChongXi_Run_JianYi()) != IS_OVER)
        return res;

    if ((res = Flows_YiJiChuShui_Run_JianYi()) != IS_OVER)
        return res;

    return 0;
}

void MeasureTask(void *pvParameters)
{
    _Measure Measure;

    Measure_Reset();
    LogDatabase_Insert("***** ���¿��� %s *****", VERSION);

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
            gWorkStatus = Measure.fid;
            gtPara.time = gtTimeBdc;
            gtPara.startTime = xTaskGetTickCount(); // ��¼��ʼʱ��;

            if (Measure.fid == 1)
            {
                LogDatabase_Insert("***** ��ʼ.ִ������ *****");
                Test_Time();
                if (Measure.id == 0)
                {
                    LogDatabase_Insert("***** ��ʼ.ִ��һ��վ���� *****");
                    Project_Run(Measure);
                }
                else if (Measure.id == 1)
                {
                    LogDatabase_Insert("***** ��ʼ.ִ�м���վ���� *****");
                    Project_Run_JianYi(Measure);
                }
                Measure_Over();
            }

            gtPara.runTotalTime = (xTaskGetTickCount() - gtPara.startTime) / 1000;
        }
    }
}
