#ifndef _APP_BUSHUI_ANDAN_H
#define _APP_BUSHUI_ANDAN_H

#define START_BUSHUI_ANDAN_EV_BIT (1 << 0) // 停止标记
#define STOP_BUSHUI_ANDAN_EV_BIT (1 << 1)  // 停止标记

int BuShuiAndan_Start(void);
int BuShuiAndan_Stop(void);
void BushuiAndanTaskCreat(void);

#endif
