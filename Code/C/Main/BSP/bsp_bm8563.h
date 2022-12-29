#ifndef _BSP_BM8563_H
#define _BSP_BM8563_H

#define BM8563_WRITE_ADDR 0xA2
#define BM8563_READ_ADDR 0xA3

#define BM8563_SECOND_ADDR 0x02
#define BM8563_Minutes_ADDR 0x03
#define BM8563_Hour_ADDR 0x04
#define BM8563_DATE_ADDR 0x05
#define BM8563_DAY_ADDR 0x06
#define BM8563_MONTH_ADDR 0x07
#define BM8563_YEAR_ADDR 0x08

void Bm8563_Init(void);
uint8_t Bm8563_Get_Time(_Time *pt_timeBCD, _Time *pt_timeBIN);
uint8_t Bm8563_Set_Time(uint32_t Time_Format, _Time t_time);

#endif
