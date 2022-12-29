#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "bsp_wifi.h"

#include <string.h>
#include "config.h"

#define WiFI_Reset  GPIO_Pin_15

#define WIFI_SIZE  256

uint8_t Wifi_Ready = 0;

#define MAX_RX_BUF_LEN 100 //the max receive buffer length, you might change it for your application
 
uint8_t uart_rx_buf[MAX_RX_BUF_LEN]; //接收缓冲区
int uart_rx_cnt; //记录接收字节数
uint8_t wifi_name_set[]="AT+WAP=11BGN,OSTD-MS-0001,CH1\r\n\0";
extern uint8_t cpuIDs[4];

void WIFI_GPIO_INIT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8;//nReset
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT; 
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;   
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;   
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz;  
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //nReload
  GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT; 
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;   
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;   
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz;  
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOA,GPIO_Pin_8);  
	GPIO_SetBits(GPIOA,GPIO_Pin_9);  

}

void Wifi_Send(int Data)
{   
	while(!USART_GetFlagStatus(WiFi_UART,USART_FLAG_TXE));	  //USART_GetFlagStatus：得到发送状态位
																															//USART_FLAG_TXE:发送寄存器为空 1：为空；0：忙状态
	USART_SendData(WiFi_UART,Data);						  							//发送一个字符
	    
}

void Wifi_Sends(const char* buf)
{
	int i;
	int size ;
	size= strlen(buf)+1; 
	for(i=0;i<size;i++)
	{
		Wifi_Send(buf[i]); 
	} 
}

void WifiReady()
{ 
	GPIO_ResetBits(GPIOD,WiFI_Reset);  
	delay_ms(300);
	GPIO_SetBits(GPIOD,WiFI_Reset);  
	Wifi_Ready =1 ;
}

uint8_t Get_WifiReady()
{
	return Wifi_Ready;
}

//让模块从透传模式进入到命令模式
int Module2CMD()
{
	int i=0;
	uart_rx_cnt = 0; // 
	memset( uart_rx_buf, 0, MAX_RX_BUF_LEN ); // 
	delay_ms(50); 
	Wifi_Send('+');  
	delay_ms(10);  
	Wifi_Send('+');
	delay_ms(10);
	Wifi_Send('+');
	for(i=0;i<300;i++)
	{
		delay_ms(10);
		uart_rx_cnt = Uart_Get(WiFi_UART, uart_rx_buf, MAX_RX_BUF_LEN);  
		if ( 'a' == uart_rx_buf[0] )
		{
			Wifi_Send('a'); // 
			//Wifi_Send('\0'); // 
			delay_ms(300);
			
			uart_rx_cnt = 0; // 
			memset( uart_rx_buf, 0, MAX_RX_BUF_LEN ); // 
			
			uart_rx_cnt = Uart_Get(WiFi_UART, uart_rx_buf, MAX_RX_BUF_LEN); 
			Log(VERBOSE,"%s,line=%d\r\n",uart_rx_buf,__LINE__); 
			return 1;
		}
		else if ( '+' == uart_rx_buf[0] && '+' == uart_rx_buf[1] && '+' == uart_rx_buf[2] )
		{
			return 1;
		}
	} 
	return 0;
}

//让模块从命令模式回到透传模式
int Module2Pass()
{ 
	uart_rx_cnt = 0;  
	memset( uart_rx_buf, 0, MAX_RX_BUF_LEN );  
	Wifi_Sends("AT+ENTM\r"); //发送ENTM命令，让模块回到透传模式
	delay_ms(200); // 
	
	uart_rx_cnt = Uart_Get(WiFi_UART, uart_rx_buf, MAX_RX_BUF_LEN);  
	if(strstr((const char *)uart_rx_buf,"ok")!=NULL)
	{ 
		return 1;
	} 
	return 0; 
}

void Wifi_Init()
{
	int i; 

  WIFI_GPIO_INIT();
  
	wifi_name_set[21] = cpuIDs[0];
	wifi_name_set[22] = cpuIDs[1];
	wifi_name_set[23] = cpuIDs[2];
	wifi_name_set[24] = cpuIDs[3];

	Log(VERBOSE,"Wifi_Init\r\n");
	
	//Wifi模块进入命令模式
	//while ( nReady ) ; //等待模块启动完成
	
	Log(VERBOSE,"Wifi nReady\r\n");
	
	i = 3; //尝试操作3次
	while ( i )
	{
		if ( Module2CMD() ) break;
		else i--;
	}

	if ( i > 0 ) //检查i的值，如果i>0,则模块已经进入命令模式，否则进入命令模式失败
	{ 
		uart_rx_cnt = 0;  
		memset( uart_rx_buf, 0, MAX_RX_BUF_LEN );  
		
		//Uart_Send(WiFi_UART, "AT+WAP\r\n\0", 33);
		Wifi_Sends("AT+WAP\r\n\0");
		delay_ms(300);
		uart_rx_cnt = Uart_Get(WiFi_UART, uart_rx_buf, MAX_RX_BUF_LEN);  
		if(strstr((const char *)uart_rx_buf,"ok")!=NULL)
		{
			if(strstr((const char *)uart_rx_buf,(const char *)(wifi_name_set+14))!=NULL)
			{
				Log(VERBOSE,"AP_NAME ok\r\n",uart_rx_buf,__LINE__);
				Log(VERBOSE,"AP_NAME ok\r\n",uart_rx_buf,__LINE__);
				goto exit;
			}
		}  
		
		Log(VERBOSE,"wifi_name_set=%s,line=%d\r\n",wifi_name_set,__LINE__);
		
		Wifi_Sends((const char *)wifi_name_set);
		delay_ms(200);
		uart_rx_cnt = Uart_Get(WiFi_UART, uart_rx_buf, MAX_RX_BUF_LEN); 
		Log(VERBOSE,"%s,line=%d\r\n",uart_rx_buf,__LINE__);
		
		Wifi_Sends("AT+WAKEY=OPEN,NONE\r\n\0");
		delay_ms(200);
		uart_rx_cnt = Uart_Get(WiFi_UART, uart_rx_buf, MAX_RX_BUF_LEN); 
		Log(VERBOSE,"%s,line=%d\r\n",uart_rx_buf,__LINE__); 
		
		Wifi_Sends("AT+LANN=192.168.1.1,255.255.255.0\r\n\0");
		delay_ms(200);
		uart_rx_cnt = Uart_Get(WiFi_UART, uart_rx_buf, MAX_RX_BUF_LEN); 
		Log(VERBOSE,"%s,line=%d\r\n",uart_rx_buf,__LINE__);
		
		Wifi_Sends("AT+NETP=TCP,Server,44444,192.168.1.1\r\n\0");
		delay_ms(200);
		uart_rx_cnt = Uart_Get(WiFi_UART, uart_rx_buf, MAX_RX_BUF_LEN); 
		Log(VERBOSE,"%s,line=%d\r\n",uart_rx_buf,__LINE__);
		
		Wifi_Sends("AT+WMODE=AP\r\n\0");
		delay_ms(200);
		uart_rx_cnt = Uart_Get(WiFi_UART, uart_rx_buf, MAX_RX_BUF_LEN); 
		Log(VERBOSE,"%s,line=%d\r\n",uart_rx_buf,__LINE__); 
	}
	else
	{
		//添加容错处理
		Log(VERBOSE,"Module2CMD fail\n");
	}

	//.........

	//让模块回到透传模式
	//while ( nReady ); //等待模块启动完成
	
	exit:
	i = 3; //尝试操作3次
	while ( i )
	{
		if ( Module2Pass() ) break;
		else i--;
	}

	if ( i <= 0 ) //检查i的值，如果i>0,则模块已经进入透传模式
	{
		//添加容错处理
		Log(VERBOSE,"Module2Pass fail\n");
	}
 
}
