#include "bsp_include.h"

#define ALARM_BUF_LEN 128
uint16_t ALARM_BUF[ALARM_BUF_LEN];

extern EventGroupHandle_t xMeasureEventGroup;

Stack_Type Alarm_Stack;
static SemaphoreHandle_t AlarmWaitSem = NULL; // ����1�����ź�������ֹ���߳�ͬʱ���ʴ���1����

void Alarm_Init(void)
{
    AlarmWaitSem = xSemaphoreCreateBinary();
    xSemaphoreGive(AlarmWaitSem);
    Stack_Init(&Alarm_Stack, ALARM_BUF, ALARM_BUF_LEN);
}

// weak = 1:����ȡ(���ݵõ�֮�󣬲�����ʧ)
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

// weak = 1:�����(���ݴ��ڼ������)
void Alarm_Add(u8 weak, u8 alarm, u8 error)
{
    int count = 0;
    uint16_t data = ((uint16_t)alarm << 8) | error;

    if (data == 0)
        return;

    xSemaphoreTake(AlarmWaitSem, 10000); // �����10��
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
        Alarm_Save(alarm, error);                                                     // ���µĹ��ϣ������߳�
        xEventGroupWaitBits(xMeasureEventGroup, STOP_EV_BIT, pdFALSE, pdFALSE, 2000); // �ȴ�һЩʱ�䣬�Ա���λ���ɼ�һЩ�澯������Ϣ
    }

    xSemaphoreGive(AlarmWaitSem);

    // һЩ���������Ҫֹͣ
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
