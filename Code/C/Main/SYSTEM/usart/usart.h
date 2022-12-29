#ifndef __USART_H
#define __USART_H
#include "stdio.h"
#include "stm32f4xx_conf.h"
#include "sys.h"
#include <stdarg.h>

#define USART1_RX_LEN 512 // 定义最大接收字节数 256
#define USART1_TX_LEN 16

#define USART2_RX_LEN 1024 // 定义最大接收字节数          256
#define USART2_TX_LEN 16

#define USART3_RX_LEN 512 // 定义最大接收字节数 256
#define USART3_TX_LEN 16

#define UART4_RX_LEN 512 // 定义最大接收字节数 256
#define UART4_TX_LEN 16

#define USART6_RX_LEN 4092 // 定义最大接收字节数 256
#define USART6_TX_LEN 16

// 如果想串口中断接收，请不要注释以下宏定义

void UART_INIT(void);
uint16_t Uart_Get(USART_TypeDef *uart, uint8_t *buffer, uint16_t len);
uint16_t Uart_Select(USART_TypeDef *uart, uint16_t delay);
void Uart_Send(USART_TypeDef *uart, const uint8_t *buffer, uint16_t len);

void uart1_init(u32 bound);
void UART1_INIT(void);
void Uart_1_Send(const uint8_t *buf, uint16_t len);
uint16_t Uart_1_Select(uint16_t delay);
uint16_t Uart_1_Get(uint8_t *buffer, uint16_t len);

void uart2_init(u32 bound);
void UART2_INIT(void);
void Uart_2_Send(const uint8_t *buf, uint16_t len);
uint16_t Uart_2_Select(uint16_t delay);
uint16_t Uart_2_Get(uint8_t *buffer, uint16_t len);

void uart3_init(u32 bound);
void UART3_INIT(void);
void Uart_3_Send(const uint8_t *buf, uint16_t len);
uint16_t Uart_3_Select(uint16_t delay);
uint16_t Uart_3_Get(uint8_t *buffer, uint16_t len);

void uart6_init(u32 bound);
void UART6_INIT(void);
void Uart_6_Send(const uint8_t *buf, uint16_t len);
uint16_t Uart_6_Select(uint16_t delay);
uint16_t Uart_6_Get(uint8_t *buffer, uint16_t len);

void uart4_init(u32 bound);
void UART4_INIT(void);
void Uart_4_Send(const uint8_t *buf, uint16_t len);
uint16_t Uart_4_Select(uint16_t delay);
uint16_t Uart_4_Get(uint8_t *buffer, uint16_t len);

void MYDMA_Send_Config(DMA_Stream_TypeDef *DMA_Streamx, u32 chx, u32 par, u32 mar, u16 ndtr); // 配置DMAx_CHx
void MYDMA_Receive_Config(DMA_Stream_TypeDef *DMA_Streamx, u32 chx, u32 par, u32 mar, u16 ndtr);
void MYDMA_Enable(const uint8_t *buf, DMA_Stream_TypeDef *DMA_Streamx, u16 ndtr); // 使能一次DMA传输

#endif
