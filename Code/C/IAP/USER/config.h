#ifndef _CONFIG_H
#define _CONFIG_H


#define VERSION_CODE  0x200
#define VERSION       "BootLoader V2.00 2020-11-05"



//ע��:Bootload�����С���ܳ���48K(ADDR_FLASH_SECTOR_0 -           ADDR_FLASH_SECTOR_2 ��������BootLoader)
//ע��:������ַ���ܳ���16K(ADDR_FLASH_SECTOR_3 �������ڱ������)
//ע��:APP��ַ�� ADDR_FLASH_SECTOR_4 ����ʼ


#define DEBUG_USART     USART1
#define WiFi_UART       USART3


//�������Ź���λʱ�� 10�� 10*(40K/256)
#define IWDG_RELOAD_TIME      1562
//���Ź�ι��Ƶ��                     (5��ιһ�ι�) 
#define IWDG_FEED_FREQ        5000



#define BOOT_FLASH_ADDR_START         0x08000000
#define BOOT_FLASH_ADDR_END           0x0800BFFF


//���õ�ַ
#define CONFIG_PARA_FLASH_ADDR        0x0800C000



//�û�APP��ַ
#define APP_CONFIG_FLASH_ADDR_START   0x0800F000
#define APP1_FLASH_ADDR_START         0x08010000


#define FLASH_SOFTWARE_LENGTH_ADDR    0X0800FFF8    //����FLASH �����ַ(����Ϊ4�ı���������ֵҪ���ڱ�������ռ��FLASH�Ĵ�С+0X08000000)
#define FLASH_SOFTWARE_CRC_ADDR       0X0800FFFC    //����FLASH �����ַ(����Ϊ4�ı���������ֵҪ���ڱ�������ռ��FLASH�Ĵ�С+0X08000000)
#define FLASH_SOFTWARE_VERSION_ADDR   0X0800FF08    //����FLASH �����ַ(����Ϊ4�ı���������ֵҪ���ڱ�������ռ��FLASH�Ĵ�С+0X08000000)


#endif

