#ifndef _APP_CMD_H
#define _APP_CMD_H


extern u8 CmdRxSemCount;


typedef union{  //�����壬ת���ֽں�ʵ����
  uint8_t b[4];
  float f;
} _FByte;

typedef union{  //�����壬ת���ֽں�ʵ����
  uint8_t b[4];
  uint16_t w[2];
  uint32_t w2;
} _W2Byte;


void CmdTask(void);


#endif


