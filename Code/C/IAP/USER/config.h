#ifndef _CONFIG_H
#define _CONFIG_H


#define VERSION_CODE  0x200
#define VERSION       "BootLoader V2.00 2020-11-05"



//注意:Bootload程序大小不能超过48K(ADDR_FLASH_SECTOR_0 -           ADDR_FLASH_SECTOR_2 区域用于BootLoader)
//注意:参数地址不能超过16K(ADDR_FLASH_SECTOR_3 区域用于保存参数)
//注意:APP地址从 ADDR_FLASH_SECTOR_4 区域开始


#define DEBUG_USART     USART1
#define WiFi_UART       USART3


//独立看门狗复位时间 10秒 10*(40K/256)
#define IWDG_RELOAD_TIME      1562
//看门狗喂狗频率                     (5秒喂一次狗) 
#define IWDG_FEED_FREQ        5000



#define BOOT_FLASH_ADDR_START         0x08000000
#define BOOT_FLASH_ADDR_END           0x0800BFFF


//配置地址
#define CONFIG_PARA_FLASH_ADDR        0x0800C000



//用户APP地址
#define APP_CONFIG_FLASH_ADDR_START   0x0800F000
#define APP1_FLASH_ADDR_START         0x08010000


#define FLASH_SOFTWARE_LENGTH_ADDR    0X0800FFF8    //设置FLASH 保存地址(必须为4的倍数，且其值要大于本代码所占用FLASH的大小+0X08000000)
#define FLASH_SOFTWARE_CRC_ADDR       0X0800FFFC    //设置FLASH 保存地址(必须为4的倍数，且其值要大于本代码所占用FLASH的大小+0X08000000)
#define FLASH_SOFTWARE_VERSION_ADDR   0X0800FF08    //设置FLASH 保存地址(必须为4的倍数，且其值要大于本代码所占用FLASH的大小+0X08000000)


#endif

