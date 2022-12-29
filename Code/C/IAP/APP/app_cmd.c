#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "iap.h"
#include "config.h"

#include "app_cmd.h"

#include <string.h>


#define RX_SIZE  1024


u8 gSysReboot = 0;

u8 CmdRxSemCount = 0;


static uint8_t RxBuffer[RX_SIZE];

static uint8_t TxBuffer[RX_SIZE];


USART_TypeDef *CMD_USART = USART1;
int rx_buffer_len;

uint32_t otaStatus=0;				//OTA״̬
uint32_t softwareVersion=0;	//�̼��汾
_W2Byte softwareLength;			//�̼���С
uint16_t _recordNumber;			//��¼��


uint16_t CRC16(uint8_t *ptr,uint16_t len);
 

void Modbus_Function_3(void)
{
	u16 reg_addr;
	u16 reg_count; 
	u16 crc16;
	u16 index = 0;
  

	TxBuffer[0] = RxBuffer[0];	//modbus��ַ
	TxBuffer[1] = 0x03;
  
	reg_addr = (RxBuffer[2]<<8) + RxBuffer[3];
	reg_count = (RxBuffer[4]<<8) + RxBuffer[5];  


	if(reg_addr == 0xFF00 && reg_count == 0x0001)								
	{ //��ȡ��ǰOTA״̬ 
    uint32_t otaStatus=1;
    index=3;
		TxBuffer[index++] = otaStatus>>8;// otaStatus>>8;
		TxBuffer[index++] = otaStatus;// otaStatus;
	}
	else if(reg_addr == 0xFF02 )
	{ //��ȡ�̼��汾
		index=3;
		Flash_Read(FLASH_SOFTWARE_VERSION_ADDR,TxBuffer+index,reg_count);
		index +=(reg_count<<1);
	}
	else if(reg_addr == 0xFF04 && reg_count == 0x0002)
	{ //��ȡ�̼���С
		_W2Byte softwareLength;
		index=3;
		Flash_Read(FLASH_SOFTWARE_LENGTH_ADDR,softwareLength.b,4);
		softwareLength.w2 += (4*1024-4);
		TxBuffer[index++] = softwareLength.b[0];
		TxBuffer[index++] = softwareLength.b[1];
		TxBuffer[index++] = softwareLength.b[2];
		TxBuffer[index++] = softwareLength.b[3];
	}
	else
	{
	  index=3;
	  TxBuffer[1] |= 0x80;
    TxBuffer[2] = 2;  //ModBusDataAddrIllegal
	}

  
  if(index>3)
	  TxBuffer[2] =  index-3; 		//�ֽ���

  
	crc16 = CRC16(TxBuffer, index);
	TxBuffer[index++] = crc16;
	TxBuffer[index++] = crc16>>8;
	Uart_Send(CMD_USART, TxBuffer, index);
}

/////////////////////////////////////////////////////////////
void Modbus_Function_6(void)
{ 
	u8 i;

	for(i = 0; i < 8; i++)
		TxBuffer[i] = RxBuffer[i];
	Uart_Send(CMD_USART, TxBuffer, 8); 
} 

/////////////////////////////////////////////////////////////
void Modbus_Function_16(void)
{
	u16 reg_addr;
	u16 reg_count;
	u16 crc16;
	u8 i; 
	
	reg_addr = (RxBuffer[2]<<8) + RxBuffer[3];
	reg_count = (RxBuffer[4]<<8) + RxBuffer[5];  

  if(reg_addr == 0xFF00 && reg_count == 0x0001)  
  { //дOTA״̬
    uint32_t otaStatus=0; 
    otaStatus = (RxBuffer[7]<<8) + RxBuffer[8];
    if(otaStatus==0){
      gSysReboot = 1;
    }    
  }
  else if(reg_addr == 0xFF02 )  
	{	//д�̼��汾
		softwareVersion = (RxBuffer[7]<<8) + RxBuffer[8];
		Flash_Write(FLASH_SOFTWARE_VERSION_ADDR,RxBuffer+7,reg_count*2);
	}  

	for(i = 0; i < 6; i++)
		TxBuffer[i] = RxBuffer[i];
	crc16 = CRC16(TxBuffer, 6);
	TxBuffer[6] = crc16;
	TxBuffer[7] = crc16>>8;
	Uart_Send(CMD_USART, TxBuffer, 8); 
} 



#define write_file_line_size 0x80

//д���ļ�
void Modbus_Function_15(void)           
{
	u8 refType;					//�ο�����
	u16 fileNumber;			//�ļ���
	u16 recordNumber;		//��¼�� 
	u16 recordLength;		//��¼����
	u16 i; 
	
	refType = (RxBuffer[3]);
	fileNumber = (RxBuffer[4]<<8) + RxBuffer[5];
	recordNumber = (RxBuffer[6]<<8) + RxBuffer[7]; 
	recordLength = (RxBuffer[8]<<8) + RxBuffer[9]; 
	 
	if(refType!=0x06)
	{
		Log(DEBUG,"д�ļ� �ο����� ERROR");
		return;
	}
	if(recordLength!=write_file_line_size)
	{
		Log(DEBUG,"д�ļ� ��¼�ų��� ERROR");		
		return;
	}
	if(softwareLength.w==0)
	{
		Log(DEBUG,"д�ļ� �̼���СΪ��");		
		return;
	}

  _recordNumber = recordNumber;
	
	Flash_Write(APP_CONFIG_FLASH_ADDR_START+_recordNumber*write_file_line_size,RxBuffer+10,recordLength);//����FLASH����   
	
	for(i = 0; i < rx_buffer_len; i++)
		TxBuffer[i] = RxBuffer[i];

  Uart_Send(CMD_USART, TxBuffer, rx_buffer_len);
} 






void CmdTask(void)
{ 
  uint16_t crc;
  uint8_t cmd; 

  if(CmdRxSemCount !=0){
    
    CMD_USART = USART1;
    rx_buffer_len = Uart_Get(CMD_USART, RxBuffer, RX_SIZE); 	//������������

    if(rx_buffer_len<5){
      CMD_USART = UART5;
      rx_buffer_len = Uart_Get(CMD_USART, RxBuffer, RX_SIZE);   //������������
    }

    if((rx_buffer_len>=5))
    {
    	crc = CRC16(RxBuffer, rx_buffer_len -2);
    	if(crc == ((RxBuffer[rx_buffer_len-1]<<8)|RxBuffer[rx_buffer_len-2]))//У��CRC
    	{
    		cmd = RxBuffer[1];
    		switch(cmd)
    		{
    			case 0x03:
    				Modbus_Function_3();
    				break;
    			case 0x06:
    				Modbus_Function_6();
    				break;
    			case 0x10:
    				Modbus_Function_16();
    				break;
    			case 0x15:
    				Modbus_Function_15();
    			break;
    		}
    	}
    }

    if(gSysReboot){
      delay_ms(1000);
      SysReboot();
    }

    CmdRxSemCount = 0;
  }
}


/* 
 * ��������CRC16
 * ����  : ����CRC16
 * ����  ��aData---���ݣ� aSize---���ݳ���
 * ���  : У��ֵ
 */
uint16_t CRC16(uint8_t *ptr,uint16_t len)
{
	 unsigned char i;
	 unsigned short crc = 0xFFFF;
	 if(len==0)
	 {
		 len = 1;
	 }
	 while(len--) 
	 {
		 crc ^= *ptr;
		 for(i=0; i<8; i++)
		 {
			 if(crc&1)
			 {
				 crc >>= 1;
				 crc ^= 0xA001;
			 }
			 else
			 {
					crc >>= 1;
			 }
		 }
		 ptr++;
	 }
	 return(crc);
}

