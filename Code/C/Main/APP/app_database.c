#include "bsp_include.h"

#define ALARM_FILE_NAME "alarm"
#define LOG_FILE_NAME "log"

static int Database_Insert(const char *fileName, void *data, int dataItemSize)
{
    FIL fsrc;
    FRESULT res;
    FSIZE_t size = 0;
    UINT len = 0;

    res = f_open(&fsrc, fileName, FA_WRITE | FA_OPEN_APPEND);
    if (res == FR_OK)
    {
        size = f_size(&fsrc);
        res |= f_lseek(&fsrc, (size + dataItemSize - 1) / dataItemSize * dataItemSize);
        res |= f_write(&fsrc, data, dataItemSize, &len);
    }

    f_close(&fsrc);
    return res;
}

#if 0
// itemIndex=0 第一条
// itemIndex=1 第二条
// itemIndex=-1 最后一条
// itemIndex=-2 倒数第二条
static int Database_Update(const char *fileName, int itemIndex, void *data, int dataItemSize)
{
    FIL fsrc;
    FRESULT res;
    FSIZE_t size = 0;
    UINT len = 0;

    res = f_open(&fsrc, fileName, FA_WRITE | FA_OPEN_EXISTING);
    if (res == FR_OK)
    {
        if (itemIndex >= 0)
        {
            res |= f_lseek(&fsrc, itemIndex * dataItemSize);
        }
        else
        {
            size = f_size(&fsrc);
            size = (size + dataItemSize - 1) / dataItemSize * dataItemSize;
            if (size + itemIndex * dataItemSize > 0)
                size += itemIndex * dataItemSize;
            else
                size = 0;

            res |= f_lseek(&fsrc, size);
        }
        res |= f_write(&fsrc, data, dataItemSize, &len);
    }

    f_close(&fsrc);
    return res;
}
#endif

// 删除整个文件
// static int Database_Delete(const char *fileName)
// {
//     return f_unlink(fileName);
// }

// 成功返回 数据库个数
// 失败返回 0
static int Database_GetCount(const char *fileName, int dataItemSize)
{
    FIL fsrc;
    FRESULT res;
    FSIZE_t size = 0;
    int count = 0;

    res = f_open(&fsrc, fileName, FA_READ | FA_OPEN_EXISTING);
    if (res == FR_OK)
    {
        size = f_size(&fsrc);
    }

    f_close(&fsrc);

    if (res == FR_OK)
    {
        count = size / dataItemSize;
    }
    else
    {
        count = 0;
    }

    return count;
}

// 返回成功读取的数据库个数
static int Database_Get(const char *fileName, int index, int count, int dataItemSize, void *data)
{
    FIL fsrc;
    FRESULT res;
    UINT len = 0;

    res = f_open(&fsrc, fileName, FA_READ | FA_OPEN_EXISTING);
    if (res == FR_OK)
    {
        res |= f_lseek(&fsrc, index * dataItemSize);
        res |= f_read(&fsrc, data, count * dataItemSize, &len);
    }

    f_close(&fsrc);

    if (res == FR_OK)
        return len / dataItemSize;
    else
        return 0;
}

static int Database_GetReverse(const char *fileName, int index, int count, int dataItemSize, void *data)
{
    FIL fsrc;
    FRESULT res;
    FSIZE_t size = 0;
    UINT len = 0;
    int seek = 0;
    int raedCount = 0;

    res = f_open(&fsrc, fileName, FA_READ | FA_OPEN_EXISTING);
    if (res == FR_OK)
    {
        size = f_size(&fsrc);
        seek = size / dataItemSize * dataItemSize;
        seek -= index * dataItemSize;

        while (count--)
        {
            seek -= dataItemSize;
            if (seek < 0)
                break;
            res |= f_lseek(&fsrc, seek);
            res |= f_read(&fsrc, data, dataItemSize, &len);
            data = (uint8_t *)data + dataItemSize;
            raedCount++;
        }
    }

    f_close(&fsrc);

    if (res == FR_OK)
        return raedCount;
    else
        return 0;
}

// 告警数据
int AlarmDatabase_Insert(_AlarmDatabase data)
{
    char fileName[32];

    sprintf(fileName, "0:/%s.bin", ALARM_FILE_NAME);
    return Database_Insert(fileName, &data, sizeof(_AlarmDatabase));
}

int AlarmDatabase_GetCount(void)
{
    char fileName[32];
    sprintf(fileName, "0:/%s.bin", ALARM_FILE_NAME);
    return Database_GetCount(fileName, sizeof(_AlarmDatabase));
}

int AlarmDatabase_GetReverse(int index, int count, _AlarmDatabase *data)
{
    char fileName[32];
    sprintf(fileName, "0:/%s.bin", ALARM_FILE_NAME);
    return Database_GetReverse(fileName, index, count, sizeof(_AlarmDatabase), data);
}

int LogDatabase_Insert(const char *format, ...)
{
    _LogDatabase data;
    memset(&data, 0, sizeof(data));
    data.time = gtTimeBdc;

    va_list ap;
    va_start(ap, format);
    vprintf(format, ap); // printf
    vsnprintf(data.info, 96, format, ap);
    va_end(ap);
    printf("\r\n");

    char fileName[32];
    sprintf(fileName, "0:/%s.bin", LOG_FILE_NAME);
    return Database_Insert(fileName, &data, sizeof(_LogDatabase));
}

int LogDatabase_GetCount(void)
{
    char fileName[32];
    sprintf(fileName, "0:/%s.bin", LOG_FILE_NAME);
    return Database_GetCount(fileName, sizeof(_LogDatabase));
}

int LogDatabase_Get(int index, int count, _LogDatabase *data)
{
    char fileName[32];
    sprintf(fileName, "0:/%s.bin", LOG_FILE_NAME);
    return Database_Get(fileName, index, count, sizeof(_LogDatabase), data);
}

int LogDatabase_GetReverse(int index, int count, _LogDatabase *data)
{
    char fileName[32];
    sprintf(fileName, "0:/%s.bin", LOG_FILE_NAME);
    return Database_GetReverse(fileName, index, count, sizeof(_LogDatabase), data);
}
