#ifndef _APP_BUSHUI_H
#define _APP_BUSHUI_H

#define START_BUSHUI_EV_BIT (1 << 0) // ֹͣ���
#define STOP_BUSHUI_EV_BIT (1 << 1)  // ֹͣ���

int ShuiXiangBuShui_Start(void);
int ShuiXiangBuShui_Stop(void);
void BushuiTaskCreat(void);

#endif
