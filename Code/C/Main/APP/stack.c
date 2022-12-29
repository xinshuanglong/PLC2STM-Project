#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "stack.h"

#include "FreeRTOS.h"
#include "semphr.h"

/*
 * 函数名：Stack_Init
 * 描述  ：初始化缓存区的Stack管理
 * 输入  ： stack---环形STACK管理结构体， buffer---指向缓存区， size---缓存区大小
 * 输出  ：注射泵状态   
 */
void Stack_Init(Stack_Type *stack, Stack_Data_Type *buffer, int size)
{
    stack->StackCtlSem = xSemaphoreCreateBinary();
    xSemaphoreGive(stack->StackCtlSem); //先释放信号量
    stack->buffer = buffer;
    stack->top = -1;
    stack->size = size;
}

/*
 * 函数名：Stack_Push
 * 描述  ：入栈，如果栈满，将丢弃前面的数据 
 * 输入  ： stack---环形STACK管理结构体， buffer---数据缓存， len---数据长度
 * 输出  ：返回实际入栈的个数
 */
int Stack_Push(Stack_Type *stack, Stack_Data_Type *buffer, int len)
{
    int i;
    s16 left = 0;

    xSemaphoreTake(stack->StackCtlSem, 500); //等待信号量，阻止多线程访问

    left = len - (stack->size - stack->top - 1); //计算需要丢弃，左移的数据个数

    if (left > 0)
    { //数据需要左移
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
 * 函数名：Stack_PushWeek
 * 描述  ：弱入栈，如果堆栈中存在一样的数据，将不执行操作 
 * 输入  ： stack---环形STACK管理结构体， buffer---数据
 * 输出  ：返回实际入栈的个数
 */
int Stack_PushWeek(Stack_Type *stack, Stack_Data_Type buffer)
{
    int i;
    int top = stack->top;

    xSemaphoreTake(stack->StackCtlSem, 500); //等待信号量，阻止多线程访问

    for (i = 0; i <= top; i++)
    {
        if (stack->buffer[i] == buffer)
        {
            xSemaphoreGive(stack->StackCtlSem);
            return 0;
        }
    }

    if (stack->top == stack->size - 1)
    { //最后一项处理
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
 * 函数名：Stack_Pop
 * 描述  ：出栈 
 * 输入  ： stack---环形STACK管理结构体， buffer---数据缓存， len---数据长度
 * 输出  ：返回实际出栈个数
 */
int Stack_Pop(Stack_Type *stack, Stack_Data_Type *buffer, int len)
{
    int n = 0;

    xSemaphoreTake(stack->StackCtlSem, 500); //等待信号量，阻止多线程访问

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
 * 函数名：Stack_PopWeak
 * 描述  ：弱出栈，数据不弹出(数据依旧保存在堆栈中) 
 * 输入  ： stack---环形STACK管理结构体， buffer---数据缓存， len---数据长度
 * 输出  ：返回实际出栈个数
 */
int Stack_PopWeak(Stack_Type *stack, Stack_Data_Type *buffer, int len)
{
    int n = 0;
    int top = stack->top;

    xSemaphoreTake(stack->StackCtlSem, 500); //等待信号量，阻止多线程访问

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
 * 函数名：Stack_Delete
 * 描述  ：删除堆栈中的元素 
 * 输入  ： stack---环形STACK管理结构体， data---要删除的数据， all:0 删除找到的第一个数据(从栈顶开始) all:1 删除找到的所有数据 
 * 输出  ：返回实际删除的个数 
 */
int Stack_Delete(Stack_Type *stack, Stack_Data_Type data, uint8_t all)
{
    int i;
    int n = 0;
    int top = stack->top;

    Stack_Data_Type buf;

    xSemaphoreTake(stack->StackCtlSem, 500); //等待信号量，阻止多线程访问

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
 * 函数名：Stack_Reset
 * 描述  ：出栈 
 * 输入  ： stack---环形STACK管理结构体， buffer---数据缓存， len---数据长度
 * 输出  ：返回实际出栈个数
 */
void Stack_Reset(Stack_Type *stack)
{
    xSemaphoreTake(stack->StackCtlSem, 500); //等待信号量，阻止多线程访问

    stack->top = -1;

    xSemaphoreGive(stack->StackCtlSem);
}
