#include "sys.h"
#include "bsp_fifo.h"

/*
 * 函数名：Fifo_Init
 * 描述  ：初始化缓存区的FIFO管理
 * 输入  ： fifo---环形FIFO管理结构体， buffer---指向缓存区， size---缓存区大小
 * 输出  ：注射泵状态   
 */
void Fifo_Init(FIFO_Type* fifo, uint8_t* buffer, uint16_t size)
{
	fifo->buffer = buffer;
	fifo->in = 0;
	fifo->out = 0;
	fifo->size = size;
}

/*
 * 函数名：Fifo_Get
 * 描述  ：获取FIFO缓存中未被读取的数据
 * 输入  ： fifo---环形FIFO管理结构体， buffer---缓存，读取FIFO中的数据写入该缓存， len---缓存最大长度
 * 输出  ：读取的数据长度
 */
uint16_t Fifo_Get(FIFO_Type* fifo, uint8_t* buffer, uint16_t len)
{
	uint16_t lenght;
	uint16_t in = fifo->in;	//防止处理过程中，中断修改fifo->in
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
 * 函数名：Fifo_Set
 * 描述  ：将数据写入到FIFO缓存中
 * 输入  ： fifo---环形FIFO管理结构体， buffer---缓存，要写入的数据缓存， len---数据长度
 * 输出  ：实际写入数据长度
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
 * 函数名：Fifo_Status
 * 描述  ：获取FIFO缓存中的未读取数据长度
 * 输入  ： fifo---环形FIFO管理结构体
 * 输出  ：未读取数据长度
 */
uint16_t Fifo_Status(FIFO_Type* fifo)
{
	uint16_t lenght;
	lenght = (fifo->in + fifo->size - fifo->out)%fifo->size;
	return lenght;
}


