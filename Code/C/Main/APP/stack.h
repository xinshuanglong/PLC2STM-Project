#ifndef __STACK_H
#define __STACK_H

typedef uint16_t Stack_Data_Type;

typedef struct
{
    Stack_Data_Type *buffer; //FIFO�Ļ�������
    int top;
    int size;          //FIFO�������鳤��
    void *StackCtlSem; //��ֹ���߳�ͬʱ���ʲ���
} Stack_Type;

void Stack_Init(Stack_Type *stack, Stack_Data_Type *buffer, int size);
int Stack_Push(Stack_Type *stack, Stack_Data_Type *buffer, int len);
int Stack_PushWeek(Stack_Type *stack, Stack_Data_Type buffer);
int Stack_Pop(Stack_Type *stack, Stack_Data_Type *buffer, int len);
int Stack_PopWeak(Stack_Type *stack, Stack_Data_Type *buffer, int len);
int Stack_Delete(Stack_Type *stack, Stack_Data_Type data, uint8_t all);
void Stack_Reset(Stack_Type *stack);

#endif
