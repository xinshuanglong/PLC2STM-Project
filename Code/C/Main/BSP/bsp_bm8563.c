#include "bsp_include.h"

typedef struct
{
    uint8_t second; // ��
    uint8_t minute; // ��
    uint8_t hour;   // ʱ
    uint8_t date;   // ��
    uint8_t week;   // ����
    uint8_t month;  // ��
    uint8_t year;   // ��
} _TimeBm8563;

void Bm8563_Init(void)
{
    IIC3_Init();
}

/*
 *   ����ֵ1 �ɹ�
 *   ����ֵ0 ʧ��
 */
uint8_t Bm8563_Data_Write(uint16_t startAddr, uint8_t *data, uint16_t count)
{
    uint16_t i;
    uint8_t res = 0;

    IIC3_Start();

    IIC3_Send_Byte(BM8563_WRITE_ADDR);
    res |= IIC3_Wait_Ack();

    IIC3_Send_Byte(startAddr);
    res |= IIC3_Wait_Ack();

    if (res)
    {
        IIC3_Stop();
        return 0;
    }

    for (i = 0; i < count; i++)
    {
        IIC3_Send_Byte(*data);
        res |= IIC3_Wait_Ack();
        if (res)
        {
            IIC3_Stop();
            return 0;
        }
        data++;
    }

    IIC3_Stop();

    return 1;
}

/*
 *   ����ֵ1 �ɹ�
 *   ����ֵ0 ʧ��
 */
uint8_t Bm8563_Data_Read(uint16_t startAddr, uint8_t *data, uint16_t count)
{
    uint16_t i;
    uint8_t res = 0;

    IIC3_Start();

    IIC3_Send_Byte(BM8563_WRITE_ADDR);
    res |= IIC3_Wait_Ack();

    IIC3_Send_Byte(startAddr);
    res |= IIC3_Wait_Ack();

    IIC3_Start();

    IIC3_Send_Byte(BM8563_READ_ADDR);
    res |= IIC3_Wait_Ack();

    if (res)
    {
        IIC3_Stop();
        return 0;
    }

    // ѭ�������һ���ǰһ��
    for (i = 0; i < count - 1; i++)
    {
        *data = IIC3_Read_Byte(1);
        data++;
    }

    // ���һ���NAck
    *data = IIC3_Read_Byte(0);

    IIC3_Stop();

    return 1;
}

/*
 *   ����ֵ1 �ɹ�
 *   ����ֵ0 ʧ��
 */
uint8_t Bm8563_Get_Time(_Time *pt_timeBCD, _Time *pt_timeBIN)
{
    uint8_t resStatus;
    _TimeBm8563 time;

    resStatus = Bm8563_Data_Read(BM8563_SECOND_ADDR, (uint8_t *)&time, 7);
    if (resStatus == 0)
        return 0;

    pt_timeBCD->year = time.year;
    pt_timeBCD->month = time.month & 0x7F;
    pt_timeBCD->date = time.date & 0x3F;
    pt_timeBCD->hour = time.hour & 0x3F;
    pt_timeBCD->minute = time.minute & 0x7F;
    pt_timeBCD->second = time.second & 0x7F;
    pt_timeBCD->week = time.week & 0x07;

    pt_timeBIN->year = BCD2Val(time.year);
    pt_timeBIN->month = BCD2Val(time.month);
    pt_timeBIN->date = BCD2Val(time.date);
    pt_timeBIN->hour = BCD2Val(time.hour);
    pt_timeBIN->minute = BCD2Val(time.minute);
    pt_timeBIN->second = BCD2Val(time.second);
    pt_timeBIN->week = BCD2Val(time.week);

    return 1;
}

/*
 *   ����ֵ1 �ɹ�
 *   ����ֵ0 ʧ��
 */
uint8_t Bm8563_Set_Time(uint32_t Time_Format, _Time t_time)
{
    _TimeBm8563 time;
    if (Time_Format == Time_Format_BIN)
    {
        time.year = Val2BCD(t_time.year);
        time.month = Val2BCD(t_time.month);
        time.date = Val2BCD(t_time.date);
        time.hour = Val2BCD(t_time.hour);
        time.minute = Val2BCD(t_time.minute);
        time.second = Val2BCD(t_time.second);
        time.week = Val2BCD(t_time.week);
    }
    else
    {
        time.year = t_time.year;
        time.month = t_time.month;
        time.date = t_time.date;
        time.hour = t_time.hour;
        time.minute = t_time.minute;
        time.second = t_time.second;
        time.week = t_time.week;
    }

    return Bm8563_Data_Write(BM8563_SECOND_ADDR, (uint8_t *)&time, 7);
}
