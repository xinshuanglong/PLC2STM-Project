#ifndef _APP_FIFO_H
#define _APP_FIFO_H

/**************** �û��������� *******************/

typedef struct
{
    uint8_t *buffer; //FIFO�Ļ�������
    uint16_t in;     //����������±꣬inָ��Ҫ�������ݵĵ�ַ
    uint16_t out;    //����������±꣬outָ��Ҫ�������ݵĵ�ַ
    uint16_t size;   //FIFO�������鳤��
} FIFO_Type;

void Fifo_Init(FIFO_Type *fifo, uint8_t *buffer, uint16_t size);

uint16_t Fifo_Get(FIFO_Type *fifo, uint8_t *buffer, uint16_t len);

uint16_t Fifo_Set(FIFO_Type *fifo, uint8_t *buffer, uint16_t len);

uint16_t Fifo_Status(FIFO_Type *fifo);

#endif //_APP_H_
