#ifndef _BSP_DS3231_H
#define _BSP_DS3231_H

#define DS3231_WRITE_ADDR 0xD0
#define DS3231_READ_ADDR 0xD1

#define DS3231_SECOND_ADDR 0x00
#define DS3231_Minutes_ADDR 0x01
#define DS3231_Hour_ADDR 0x02
#define DS3231_DAY_ADDR 0x03
#define DS3231_DATE_ADDR 0x04
#define DS3231_MONTH_ADDR 0x05
#define DS3231_YEAR_ADDR 0x06

void Ds3231_Init(void);
uint8_t Ds3231_Get_Time(_Time *pt_timeBCD, _Time *pt_timeBIN);
uint8_t Ds3231_Set_Time(uint32_t Time_Format, _Time t_time);

#endif
