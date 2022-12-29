#ifndef BSP_WIFI_H
#define BSP_WIFI_H 

//#define AP_NAME		"OSTD-MS-D"
#define nReady 		GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_5) //PE5

void WIFI_GPIO_INIT(void);
void Wifi_Send(int Data);
void Wifi_Sends(const char* buf);
void WifiReady(void);
void Wifi_AP_Init(void);
void Wifi_Init(void);
int Module2CMD(void);
int Module2Pass(void);



#endif
