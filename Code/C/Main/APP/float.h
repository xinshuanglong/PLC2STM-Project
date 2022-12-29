#ifndef _FLOAT_H
#define _FLOAT_H

//#define BigEndian 1   //大端端模式
#define SmallEndian 1 //小端模式

bool isInf(float d);
bool isNan(float d);
bool isIllegal(float d);
bool isFinite(float d);

#endif
