#ifndef _APP_BUSHUI_H
#define _APP_BUSHUI_H

#define START_BUSHUI_EV_BIT (1 << 0) // 停止标记
#define STOP_BUSHUI_EV_BIT (1 << 1)  // 停止标记

int ShuiXiangBuShui_Start(void);
int ShuiXiangBuShui_Stop(void);
void BushuiTaskCreat(void);

#endif
