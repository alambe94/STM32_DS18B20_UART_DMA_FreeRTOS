/*
 * print_temp_thread.c
 *
 *  Created on: Jul 30, 2019
 *      Author: a
 */

#include "main.h"
#include "RTOS.h"
#include "stdlib.h"
#include "string.h"
#include "printf.h"


#define          PRINT_TASK_STACK_SIZE 256u
#define          PRINT_TASK_PRIORITY   4u
OS_STACKPTR int  Print_Task_Stack[PRINT_TASK_STACK_SIZE];
OS_TASK          Print_Task_TCB;
static void      Print_Task();

OS_MUTEX         UART_Mutex_Handle;

extern OS_QUEUE  DS18B20_Q;


extern UART_HandleTypeDef huart2;
UART_HandleTypeDef* CLI_UART = &huart2;

void Print_Thread_Add()
    {

    OS_MUTEX_Create(&UART_Mutex_Handle);

    OS_TASK_CREATE(&Print_Task_TCB, "Print_Task", 80, Print_Task, Print_Task_Stack);

    }

void CLI_UART_Send_Char(const char data)
    {

    CLI_UART->Instance->DR = (data);
    while (__HAL_UART_GET_FLAG(CLI_UART,UART_FLAG_TC) == 0);

    }

void CLI_UART_Send_String(const char* data)
    {

    /*** gaurd uart ***/
    OS_MUTEX_LockBlocked(&UART_Mutex_Handle);

    uint16_t count = 0;
    while (*data)
	{
	CLI_UART_Send_Char(*data++);
	count++;
	}

    /*** release uart ***/
    OS_MUTEX_Unlock(&UART_Mutex_Handle);
    }

void CLI_UART_Send_String_DMA(const char* data)
    {

    /*** gaurd uart ***/
    OS_MUTEX_LockBlocked(&UART_Mutex_Handle);

    HAL_UART_Transmit_DMA(CLI_UART, (uint8_t*) data, strlen(data));

    /*** release uart ***/
    OS_MUTEX_Unlock(&UART_Mutex_Handle);

    }

void CLI_UART_Send_Int(int32_t num)
    {

    char int_to_str[10] ={0};

    itoa(num, int_to_str, 10);
    CLI_UART_Send_String(int_to_str);

    }

void CLI_UART_Send_Float(float num)
    {
    char int_to_str[10] =
	{
	0
	};
    sprintf(int_to_str, "%0.2f", num);
    CLI_UART_Send_String(int_to_str);
    }



static void Print_Task()
    {

    uint16_t* q_data = NULL;
    float temperature = 0.0;

    while (1)
	{

	OS_QUEUE_GetPtrBlocked(&DS18B20_Q, (void**) &q_data);

	if (q_data != NULL)
	    {
	    temperature = (float) *q_data / (float) 16;
	    CLI_UART_Send_Float(temperature);
	    CLI_UART_Send_String("\r\n");
	    }
	OS_QUEUE_Purge(&DS18B20_Q);
	}

    }
