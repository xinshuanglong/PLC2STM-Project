#include "bsp_include.h"

bool isInf(float d)
{
    uint8_t *ch = (uint8_t *)&d;
#if BigEndian
    return (bool)((ch[0] & 0x7f) == 0x7f && ch[1] == 0x80);
#else
    return (bool)((ch[3] & 0x7f) == 0x7f && ch[2] == 0x80);
#endif
}

bool isNan(float d)
{
    uint8_t *ch = (uint8_t *)&d;
#if BigEndian

    return (bool)((ch[0] & 0x7f) == 0x7f && ch[1] > 0x80);
#else

    return (bool)((ch[3] & 0x7f) == 0x7f && ch[2] > 0x80);
#endif
}

//isInf + isNan
bool isIllegal(float d)
{
    uint8_t *ch = (uint8_t *)&d;
#if BigEndian
    return (bool)((ch[0] & 0x7f) == 0x7f && ch[1] >= 0x80);
#else
    return (bool)((ch[3] & 0x7f) == 0x7f && ch[2] >= 0x80);
#endif
}

bool isFinite(float d)
{
    uint8_t *ch = (uint8_t *)&d;
#if BigEndian

    return (bool)((ch[0] & 0x7f) != 0x7f || (ch[1] & 0x80) != 0x80);
#else

    return (bool)((ch[3] & 0x7f) != 0x7f || (ch[2] & 0x80) != 0x80);
#endif
}
