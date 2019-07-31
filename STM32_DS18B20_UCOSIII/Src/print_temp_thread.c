/*
 * print_temp_thread.c
 *
 *  Created on: Jul 30, 2019
 *      Author: a
 */

#include "main.h"
#include "os.h"
#include "stdlib.h"
#include "string.h"
#include "printf.h"


#define      PRINT_TASK_STACK_SIZE 256u
#define      PRINT_TASK_PRIORITY   4u
CPU_STK      Print_Task_Stack[PRINT_TASK_STACK_SIZE];
OS_TCB       Print_Task_TCB;
static void  Print_Task(void* argument);

OS_MUTEX     UART_Mutex_Handle;

extern OS_Q  DS18B20_Q;


extern UART_HandleTypeDef huart2;
UART_HandleTypeDef* CLI_UART = &huart2;

void Print_Thread_Add()
    {

    OS_ERR  os_err;

    OSMutexCreate((OS_MUTEX*)&UART_Mutex_Handle,
    		  (CPU_CHAR*)"UART_Mutex",
    		  (OS_ERR*	) &os_err);

    OSTaskCreate(&Print_Task_TCB,
                      "Print_Task",
		      Print_Task,
                      0u,
		      PRINT_TASK_PRIORITY,
                      &Print_Task_Stack[0u],
		      Print_Task_Stack[PRINT_TASK_STACK_SIZE / 10u],
		      PRINT_TASK_STACK_SIZE,
                      0u,
                      0u,
                      0u,
                      (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                      &os_err);

    }

void CLI_UART_Send_Char(const char data)
    {

    CLI_UART->Instance->DR = (data);
    while (__HAL_UART_GET_FLAG(CLI_UART,UART_FLAG_TC) == 0);

    }

void CLI_UART_Send_String(const char* data)
    {

    OS_ERR  os_err;

    /*** gaurd uart ***/
    OSMutexPend(&UART_Mutex_Handle, 0, OS_OPT_PEND_BLOCKING, 0, &os_err);

    uint16_t count = 0;
    while (*data)
	{
	CLI_UART_Send_Char(*data++);
	count++;
	}

    /*** release uart ***/
    OSMutexPost(&UART_Mutex_Handle, OS_OPT_POST_NONE, &os_err);

    }

void CLI_UART_Send_String_DMA(const char* data)
    {

    OS_ERR  os_err;

    /*** gaurd uart ***/
    OSMutexPend(&UART_Mutex_Handle, 0, OS_OPT_PEND_BLOCKING, 0, &os_err);

    HAL_UART_Transmit_DMA(CLI_UART, (uint8_t*) data, strlen(data));

    /*** release uart ***/
    OSMutexPost(&UART_Mutex_Handle, OS_OPT_POST_NONE, &os_err);

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



static void Print_Task(void* argument)
    {

    OS_ERR os_err;
    uint16_t* q_data = 0;
    float temperature = 0.0;
    OS_MSG_SIZE msg_size;
    CPU_TS ts;

    while (1)
	{

	//check if valid temperature is received from ds18b20 task within one second

	q_data = OSQPend(&DS18B20_Q, 1000, OS_OPT_PEND_BLOCKING, &msg_size, &ts,
		&os_err);

	if (q_data != NULL)
	    {
	    temperature = (float) *q_data / (float) 16;
	    CLI_UART_Send_Float(temperature);
	    CLI_UART_Send_String("\r\n");
	    }

	}

    }
