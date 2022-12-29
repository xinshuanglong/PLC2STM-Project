#ifndef _APP_PORT_H
#define _APP_PORT_H

void PortTaskCreat(void);

extern u32 gIoStateInBit[IO_INDEX_COUNT / 32];
extern bool gIoState[IO_INDEX_COUNT];

extern volatile _Board gBoardOpt;

#endif
