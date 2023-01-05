#include "bsp_include.h"

#define ALARM_BUF_LEN 128
uint16_t ALARM_BUF[ALARM_BUF_LEN];

extern EventGroupHandle_t xMeasureEventGroup;

Stack_Type Alarm_Stack;
static SemaphoreHandle_t AlarmWaitSem = NULL; // 串口1发送信号量，阻止多线程同时访问串口1发送

void Alarm_Init(void)
{
    AlarmWaitSem = xSemaphoreCreateBinary();
    xSemaphoreGive(AlarmWaitSem);
    Stack_Init(&Alarm_Stack, ALARM_BUF, ALARM_BUF_LEN);
}

// weak = 1:弱获取(数据得到之后，并不消失)
void Alarm_Get(u8 weak, u8 *alarm, u8 *error)
{
    uint16_t data = 0;

    if (!weak)
    {
        Stack_Pop(&Alarm_Stack, &data, 1);
    }
    else
    {
        Stack_PopWeak(&Alarm_Stack, &data, 1);
    }

    *alarm = (data >> 8) & 0xFF;
    *error = data & 0xFF;
}

// weak = 1:弱添加(数据存在即不添加)
void Alarm_Add(u8 weak, u8 alarm, u8 error)
{
    int count = 0;
    uint16_t data = ((uint16_t)alarm << 8) | error;

    if (data == 0)
        return;

    xSemaphoreTake(AlarmWaitSem, 10000); // 最长阻塞10秒
    if (!weak)
    {
        count = Stack_Push(&Alarm_Stack, &data, 1);
    }
    else
    {
        count = Stack_PushWeek(&Alarm_Stack, data);
    }

    if (count != 0)
    {
        Alarm_Save(alarm, error);                                                     // 有新的故障，阻塞线程
        xEventGroupWaitBits(xMeasureEventGroup, STOP_EV_BIT, pdFALSE, pdFALSE, 2000); // 等待一些时间，以便上位机采集一些告警错误信息
    }

    xSemaphoreGive(AlarmWaitSem);

    // 一些特殊情况需要停止
}

void Alarm_Delete(u8 alarm, u8 error)
{
    uint16_t data = ((uint16_t)alarm << 8) | error;

    Stack_Delete(&Alarm_Stack, data, 1);
}

void Alarm_Reset(void)
{
    Stack_Reset(&Alarm_Stack);
}
