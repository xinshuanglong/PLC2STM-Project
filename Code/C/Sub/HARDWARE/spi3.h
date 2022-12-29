#ifndef _SPI3_H
#define _SPI3_H

void SPI3_Init(void);             //初始化SPI3口
void SPI3_SetSpeed(u8 SpeedSet);  //设置SPI3速度
u8 SPI3_ReadWriteByte(u8 TxData); //SPI3总线读写一个字节

#endif
