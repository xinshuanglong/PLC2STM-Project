#ifndef _APP_BUSHUI_ANDAN_H
#define _APP_BUSHUI_ANDAN_H

#define START_BUSHUI_ANDAN_EV_BIT (1 << 0) // ֹͣ���
#define STOP_BUSHUI_ANDAN_EV_BIT (1 << 1)  // ֹͣ���

int BuShuiAndan_Start(void);
int BuShuiAndan_Stop(void);
int BuShuiAndan_Start_JianYi(void);
int BuShuiAndan_Stop_JianYi(void);

void BushuiAndanTaskCreat(void);

#endif
