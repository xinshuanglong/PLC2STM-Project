#ifndef _APP_EV_BIT_H
#define _APP_EV_BIT_H

#define STOP_EV_BIT (1 << 0)       //停止标记
#define ERROR_EV_BIT (1 << 1)      //故障类错误，需要停止(例如加热)
#define CLEAN_OVER_EV_BIT (1 << 2) //清洗完成

#endif
