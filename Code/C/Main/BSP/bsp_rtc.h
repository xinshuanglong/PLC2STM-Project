#ifndef _BSP_RTC_H
#define _BSP_RTC_H

#define Time_Format_BIN ((uint32_t)0x000000000)
#define Time_Format_BCD ((uint32_t)0x000000001)

void MY_RTC_Init(void);

uint8_t RTC_Get_Time(_Time *pt_timeBCD, _Time *pt_timeBIN);
uint8_t RTC_Set_Time(uint32_t Time_Format, _Time t_time);

void RTC_Time_To_Data(_Time t_time, uint8_t *data);
void Data_TO_RTC_Time(_Time *t_time, uint8_t *data);

uint8_t BCD2Val(uint8_t Value);
uint8_t Val2BCD(uint8_t Value);

#endif
