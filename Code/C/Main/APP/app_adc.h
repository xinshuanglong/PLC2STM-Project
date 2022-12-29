#ifndef _APP_ADC_H
#define _APP_ADC_H

extern float gAdcRef, gVcc;
extern float gAdcKfp[ADC_Channel_Count_Without_Refint];

void AdcTaskCreat(void);

#endif
