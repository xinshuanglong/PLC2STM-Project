#ifndef _APP_PORT_H
#define _APP_PORT_H

void PortTaskCreat(void);

extern volatile u32 gIoStateInBit[IO_INDEX_COUNT / 32];
extern volatile bool gIoState[IO_INDEX_COUNT];
extern volatile _Board gBoardState;

bool Port_Read_Filter(int index);

#endif
