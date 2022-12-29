#include "bsp_include.h"

// #define RTC_DS3231 1
#define RTC_BM8563 1

static SemaphoreHandle_t RTCOptSem = NULL;

void MY_RTC_Init(void)
{
#ifdef RTC_DS3231
    Ds3231_Init();
#endif
#ifdef RTC_BM8563
    Bm8563_Init();
#endif

    RTCOptSem = xSemaphoreCreateBinary();
    xSemaphoreGive(RTCOptSem); // 释放信号量
}

/*
 *   返回值1 成功
 *   返回值0 失败
 */
uint8_t RTC_Get_Time(_Time *pt_timeBCD, _Time *pt_timeBIN)
{
    uint8_t resStatus = 0;

    xSemaphoreTake(RTCOptSem, 1000); // 阻止多线程访问
#ifdef RTC_DS3231
    resStatus = Ds3231_Get_Time(pt_timeBCD, pt_timeBIN);
#endif
#ifdef RTC_BM8563
    resStatus = Bm8563_Get_Time(pt_timeBCD, pt_timeBIN);
#endif
    xSemaphoreGive(RTCOptSem); // 阻止多线程访问
    if (resStatus == 0)
        return 0;
    else
        return 1;
}

/*
 *   返回值1 成功
 *   返回值0 失败
 */
uint8_t RTC_Set_Time(uint32_t Time_Format, _Time t_time)
{
    uint8_t resStatus = 0;

    xSemaphoreTake(RTCOptSem, 1000); // 阻止多线程访问
#ifdef RTC_DS3231
    resStatus = Ds3231_Set_Time(Time_Format, t_time);
#endif
#ifdef RTC_BM8563
    resStatus = Bm8563_Set_Time(Time_Format, t_time);
#endif
    xSemaphoreGive(RTCOptSem); // 阻止多线程访问

    if (resStatus == 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

uint8_t BCD2Val(uint8_t Value)
{
    uint8_t tmp = 0;
    tmp = ((uint8_t)(Value & (uint8_t)0xF0) >> (uint8_t)0x4) * 10;
    return (tmp + (Value & (uint8_t)0x0F));
}

uint8_t Val2BCD(uint8_t Value)
{
    uint8_t bcdhigh = 0;

    while (Value >= 10)
    {
        bcdhigh++;
        Value -= 10;
    }

    return ((uint8_t)(bcdhigh << 4) | Value);
}

void RTC_Time_To_Data(_Time t_time, uint8_t *data)
{
    data[0] = t_time.year;
    data[1] = t_time.month;
    data[2] = t_time.date;
    data[3] = t_time.hour;
    data[4] = t_time.minute;
    data[5] = t_time.second;
}

void Data_TO_RTC_Time(_Time *t_time, uint8_t *data)
{
    t_time->year = data[0];
    t_time->month = data[1];
    t_time->date = data[2];
    t_time->hour = data[3];
    t_time->minute = data[4];
    t_time->second = data[5];
}
