#ifndef __ALARM_H
#define __ALARM_H

void Alarm_Init(void);
void Alarm_Get(u8 weak, u8 *alarm, u8 *error);
void Alarm_Add(u8 weak, u8 alarm, u8 error);
void Alarm_Delete(u8 alarm, u8 error);
void Alarm_Reset(void);

extern char gDataIdentification[12];
extern volatile bool gErrorFlag;

#endif
