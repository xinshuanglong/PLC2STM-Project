#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "stack.h"

#include "FreeRTOS.h"
#include "semphr.h"

/*
 * ��������Stack_Init
 * ����  ����ʼ����������Stack����
 * ����  �� stack---����STACK����ṹ�壬 buffer---ָ�򻺴����� size---��������С
 * ���  ��ע���״̬   
 */
void Stack_Init(Stack_Type *stack, Stack_Data_Type *buffer, int size)
{
    stack->StackCtlSem = xSemaphoreCreateBinary();
    xSemaphoreGive(stack->StackCtlSem); //���ͷ��ź���
    stack->buffer = buffer;
    stack->top = -1;
    stack->size = size;
}

/*
 * ��������Stack_Push
 * ����  ����ջ�����ջ����������ǰ������� 
 * ����  �� stack---����STACK����ṹ�壬 buffer---���ݻ��棬 len---���ݳ���
 * ���  ������ʵ����ջ�ĸ���
 */
int Stack_Push(Stack_Type *stack, Stack_Data_Type *buffer, int len)
{
    int i;
    s16 left = 0;

    xSemaphoreTake(stack->StackCtlSem, 500); //�ȴ��ź�������ֹ���̷߳���

    left = len - (stack->size - stack->top - 1); //������Ҫ���������Ƶ����ݸ���

    if (left > 0)
    { //������Ҫ����
        for (i = 0; i < stack->size - left; i++)
        {
            stack->buffer[i] = stack->buffer[i + left];
        }
        stack->top -= left;
    }

    for (i = 0; i < len; i++)
    {
        stack->buffer[(stack->top) + 1] = *buffer;
        stack->top++;
        buffer++;
    }

    xSemaphoreGive(stack->StackCtlSem);

    return len;
}

/*
 * ��������Stack_PushWeek
 * ����  ������ջ�������ջ�д���һ�������ݣ�����ִ�в��� 
 * ����  �� stack---����STACK����ṹ�壬 buffer---����
 * ���  ������ʵ����ջ�ĸ���
 */
int Stack_PushWeek(Stack_Type *stack, Stack_Data_Type buffer)
{
    int i;
    int top = stack->top;

    xSemaphoreTake(stack->StackCtlSem, 500); //�ȴ��ź�������ֹ���̷߳���

    for (i = 0; i <= top; i++)
    {
        if (stack->buffer[i] == buffer)
        {
            xSemaphoreGive(stack->StackCtlSem);
            return 0;
        }
    }

    if (stack->top == stack->size - 1)
    { //���һ���
        for (i = 0; i < stack->size - 1; i++)
        {
            stack->buffer[i] = stack->buffer[i + 1];
        }
        stack->top -= 1;
    }

    stack->buffer[(stack->top) + 1] = buffer;
    stack->top++;

    xSemaphoreGive(stack->StackCtlSem);

    return 1;
}

/*
 * ��������Stack_Pop
 * ����  ����ջ 
 * ����  �� stack---����STACK����ṹ�壬 buffer---���ݻ��棬 len---���ݳ���
 * ���  ������ʵ�ʳ�ջ����
 */
int Stack_Pop(Stack_Type *stack, Stack_Data_Type *buffer, int len)
{
    int n = 0;

    xSemaphoreTake(stack->StackCtlSem, 500); //�ȴ��ź�������ֹ���̷߳���

    while (len-- && stack->top > -1)
    {
        *buffer = stack->buffer[stack->top--];
        buffer++;
        n++;
    }

    xSemaphoreGive(stack->StackCtlSem);

    return n;
}

/*
 * ��������Stack_PopWeak
 * ����  ������ջ�����ݲ�����(�������ɱ����ڶ�ջ��) 
 * ����  �� stack---����STACK����ṹ�壬 buffer---���ݻ��棬 len---���ݳ���
 * ���  ������ʵ�ʳ�ջ����
 */
int Stack_PopWeak(Stack_Type *stack, Stack_Data_Type *buffer, int len)
{
    int n = 0;
    int top = stack->top;

    xSemaphoreTake(stack->StackCtlSem, 500); //�ȴ��ź�������ֹ���̷߳���

    while (len-- && top > -1)
    {
        *buffer = stack->buffer[top--];
        buffer++;
        n++;
    }

    xSemaphoreGive(stack->StackCtlSem);

    return n;
}

/*
 * ��������Stack_Delete
 * ����  ��ɾ����ջ�е�Ԫ�� 
 * ����  �� stack---����STACK����ṹ�壬 data---Ҫɾ�������ݣ� all:0 ɾ���ҵ��ĵ�һ������(��ջ����ʼ) all:1 ɾ���ҵ����������� 
 * ���  ������ʵ��ɾ���ĸ��� 
 */
int Stack_Delete(Stack_Type *stack, Stack_Data_Type data, uint8_t all)
{
    int i;
    int n = 0;
    int top = stack->top;

    Stack_Data_Type buf;

    xSemaphoreTake(stack->StackCtlSem, 500); //�ȴ��ź�������ֹ���̷߳���

    while (top > -1)
    {
        buf = stack->buffer[top];

        if (buf == data)
        {
            for (i = top; i < stack->top; i++)
            {
                stack->buffer[i] = stack->buffer[i + 1];
            }
            n++;
            stack->top--;
            if (all == 0)
                break;
        }
        top--;
    }

    xSemaphoreGive(stack->StackCtlSem);

    return n;
}

/*
 * ��������Stack_Reset
 * ����  ����ջ 
 * ����  �� stack---����STACK����ṹ�壬 buffer---���ݻ��棬 len---���ݳ���
 * ���  ������ʵ�ʳ�ջ����
 */
void Stack_Reset(Stack_Type *stack)
{
    xSemaphoreTake(stack->StackCtlSem, 500); //�ȴ��ź�������ֹ���̷߳���

    stack->top = -1;

    xSemaphoreGive(stack->StackCtlSem);
}
