#ifndef _APP_DATABASE_H
#define _APP_DATABASE_H

#pragma pack(1)

typedef struct
{
    _Time time;
    char unUsed;
    char unUsed2[24];
    char info[96]; // info放在最后面
} _LogDatabase;    // 大小128Byte

typedef struct
{
    union
    {
        struct
        {
            _Time time;
            uint8_t id;
            uint8_t fid;
            uint16_t step;
            char flowDescribe[16]; // 当前步骤流程中文名字
            uint8_t alarm;
            uint8_t error;
        };
        char unUsed[64];
    };
} _AlarmDatabase;
#pragma pack()

int LogDatabase_Insert(const char *format, ...);
int LogDatabase_GetCount(void);
int LogDatabase_Get(int index, int count, _LogDatabase *data);
int LogDatabase_GetReverse(int index, int count, _LogDatabase *data);

#endif
