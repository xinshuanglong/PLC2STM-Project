#ifndef _FLOAT_H
#define _FLOAT_H

//#define BigEndian 1   //��˶�ģʽ
#define SmallEndian 1 //С��ģʽ

bool isInf(float d);
bool isNan(float d);
bool isIllegal(float d);
bool isFinite(float d);

#endif
