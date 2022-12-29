#ifndef _APP_CMD_H
#define _APP_CMD_H


extern u8 CmdRxSemCount;


typedef union{  //共用体，转换字节和实数。
  uint8_t b[4];
  float f;
} _FByte;

typedef union{  //共用体，转换字节和实数。
  uint8_t b[4];
  uint16_t w[2];
  uint32_t w2;
} _W2Byte;


void CmdTask(void);


#endif


