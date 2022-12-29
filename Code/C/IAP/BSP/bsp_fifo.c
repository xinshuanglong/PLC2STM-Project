#include "sys.h"
#include "bsp_fifo.h"

/*
 * ��������Fifo_Init
 * ����  ����ʼ����������FIFO����
 * ����  �� fifo---����FIFO����ṹ�壬 buffer---ָ�򻺴����� size---��������С
 * ���  ��ע���״̬   
 */
void Fifo_Init(FIFO_Type* fifo, uint8_t* buffer, uint16_t size)
{
	fifo->buffer = buffer;
	fifo->in = 0;
	fifo->out = 0;
	fifo->size = size;
}

/*
 * ��������Fifo_Get
 * ����  ����ȡFIFO������δ����ȡ������
 * ����  �� fifo---����FIFO����ṹ�壬 buffer---���棬��ȡFIFO�е�����д��û��棬 len---������󳤶�
 * ���  ����ȡ�����ݳ���
 */
uint16_t Fifo_Get(FIFO_Type* fifo, uint8_t* buffer, uint16_t len)
{
	uint16_t lenght;
	uint16_t in = fifo->in;	//��ֹ��������У��ж��޸�fifo->in
	uint16_t i;
	lenght = (in + fifo->size - fifo->out)%fifo->size;
	if(lenght > len)
		lenght = len;
	for(i = 0; i < lenght; i++)
	{
		buffer[i] = fifo->buffer[(fifo->out + i)%fifo->size];
	}
	fifo->out = (fifo->out + lenght)%fifo->size;
	return lenght;
}

/*
 * ��������Fifo_Set
 * ����  ��������д�뵽FIFO������
 * ����  �� fifo---����FIFO����ṹ�壬 buffer---���棬Ҫд������ݻ��棬 len---���ݳ���
 * ���  ��ʵ��д�����ݳ���
 */
uint16_t Fifo_Set(FIFO_Type* fifo, uint8_t* buffer, uint16_t len)
{
	uint16_t lenght;
	uint16_t i;
	lenght = (fifo->out + fifo->size - fifo->in)%fifo->size;
	if(lenght > len)
		lenght = len;
	for(i = 0; i < lenght; i++)
	{
		fifo->buffer[(fifo->in + i)%fifo->size] = buffer[i];
	}
	fifo->in = (fifo->in + lenght)%fifo->size;
	return lenght;
}

/*
 * ��������Fifo_Status
 * ����  ����ȡFIFO�����е�δ��ȡ���ݳ���
 * ����  �� fifo---����FIFO����ṹ��
 * ���  ��δ��ȡ���ݳ���
 */
uint16_t Fifo_Status(FIFO_Type* fifo)
{
	uint16_t lenght;
	lenght = (fifo->in + fifo->size - fifo->out)%fifo->size;
	return lenght;
}


