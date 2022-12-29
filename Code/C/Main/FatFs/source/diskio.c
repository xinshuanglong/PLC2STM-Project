/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"     /* Obtains integer types */
#include "diskio.h" /* Declarations of disk functions */
#include "bsp_include.h"

/* Definitions of physical drive number for each drive */
// #define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
// #define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
// #define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */
#define DEV_MMC 0

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(
    BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
    switch (pdrv)
    {
    case DEV_MMC:
        return RES_OK;
    }
    return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status_my(
    BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
    uint32_t result;

    switch (pdrv)
    {
    case DEV_MMC:
#if FF_FS_REENTRANT
        if (!ff_mutex_take(DEV_MMC))
            return FR_TIMEOUT;
#endif
        result = SD_GetState();
#if FF_FS_REENTRANT
        ff_mutex_give(DEV_MMC);
#endif
        break;
    default:
        result = SD_CARD_ERROR;
        break;
    }

    if (result == SD_CARD_ERROR)
        return STA_NODISK;
    else
        return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
    BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
    int result;

    switch (pdrv)
    {
    case DEV_MMC:
        result = SD_Init();
        break;
    default:
        result = RES_PARERR;
        break;
    }

    if (result)
        return STA_NOINIT;
    else
        return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
    BYTE pdrv,    /* Physical drive nmuber to identify the drive */
    BYTE *buff,   /* Data buffer to store read data */
    LBA_t sector, /* Start sector in LBA */
    UINT count    /* Number of sectors to read */
)
{
    int result;

    if (!count)
        return RES_PARERR; // count不能等于0，否则返回参数错误

    switch (pdrv)
    {
    case DEV_MMC:
        result = SD_ReadDisk(buff, sector, count);
        break;
    default:
        result = RES_PARERR;
        break;
    }

    if (result)
        return RES_ERROR;
    else
        return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(
    BYTE pdrv,        /* Physical drive nmuber to identify the drive */
    const BYTE *buff, /* Data to be written */
    LBA_t sector,     /* Start sector in LBA */
    UINT count        /* Number of sectors to write */
)
{
    int result;

    if (!count)
        return RES_PARERR; // count不能等于0，否则返回参数错误

    switch (pdrv)
    {
    case DEV_MMC:
        result = SD_WriteDisk((u8 *)buff, sector, count);
        break;
    default:
        result = RES_PARERR;
        break;
    }

    if (result)
        return RES_ERROR;
    else
        return RES_OK;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(
    BYTE pdrv, /* Physical drive nmuber (0..) */
    BYTE cmd,  /* Control code */
    void *buff /* Buffer to send/receive control data */
)
{
    DRESULT res;
    if (pdrv == DEV_MMC) // SD卡
    {
        switch (cmd)
        {
        case CTRL_SYNC:
            res = RES_OK;
            break;
        case GET_SECTOR_SIZE:
            *(DWORD *)buff = 512;
            res = RES_OK;
            break;
        case GET_BLOCK_SIZE:
            *(WORD *)buff = 1;
            res = RES_OK;
            break;
        case GET_SECTOR_COUNT:
            *(DWORD *)buff = 8192;
            res = RES_OK;
            break;
        default:
            res = RES_PARERR;
            break;
        }
    }
    else
        res = RES_PARERR; // 其他的不支持
    return res;
}

// User defined function to give a current time to fatfs module      */
// 31-25: Year(0-127 org.1980), 24-21: Month(1-12), 20-16: Day(1-31) */
// 15-11: Hour(0-23), 10-5: Minute(0-59), 4-0: Second(0-29 *2) */
DWORD get_fattime(void)
{
    u32 fattime;
    _Time timer;
    u32 year;
    u32 month;
    u32 day;
    u32 hour;
    u32 min;
    u32 sec;
    timer = gtTimeBin;
    sec = (u32)timer.second / 2;
    min = (u32)timer.minute << 5;
    hour = (u32)timer.hour << 11;
    day = (u32)timer.date << 16;
    month = (u32)timer.month << 21;
    year = ((u32)timer.year + 2000 - 1980) << 25;
    fattime = year | month | day | hour | min | sec;
    return fattime;
}
