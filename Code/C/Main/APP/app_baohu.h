#ifndef _APP_BAOHU_H
#define _APP_BAOHU_H

#define START_BAOHU_EV_BIT (1 << 0) // ֹͣ���
#define STOP_BAOHU_EV_BIT (1 << 1)  // ֹͣ���

int Baohu_Start(void);
int Baohu_Stop(void);
void BaohuTaskCreat(void);

#endif
