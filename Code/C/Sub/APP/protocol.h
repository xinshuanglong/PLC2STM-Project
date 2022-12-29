#ifndef _PROTOCOL_H
#define _PROTOCOL_H

typedef union
{ // 共用体，转换字节和实数。
    uint8_t b[4];
    uint16_t w[2];
    uint32_t w2;
} _W2Byte;

typedef enum
{
    ModBusNoError = 0,
    ModBusFunIllegal = 1,
    ModBusDataAddrIllegal,
    ModBusDataIllegal,
    ModBusDeviceFault,
    ModBusConfirm,
    ModBusDeviceBusy
} ModBusError;

uint16_t ModBus_Protocol(uint8_t *data, uint16_t len, uint8_t *backData);

#endif
