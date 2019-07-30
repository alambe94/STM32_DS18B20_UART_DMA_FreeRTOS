/*
 * print_temp_thread.c
 *
 *  Created on: Jul 30, 2019
 *      Author: a
 */

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "stdlib.h"
#include "string.h"
#include "printf.h"


#define      PRINT_TASK_STACK_SIZE 256u
#define      PRINT_TASK_PRIORITY   4u
TaskHandle_t Print_Task_Handle;
StackType_t  Print_Task_Stack[PRINT_TASK_STACK_SIZE];
StaticTask_t Print_Task_TCB;
static void  Print_Task(void* argument);

SemaphoreHandle_t UART_Mutex_Handle;
StaticSemaphore_t UART_Mutex_Buffer;


extern QueueHandle_t DS18B20_Q;


extern UART_HandleTypeDef huart2;
UART_HandleTypeDef* CLI_UART = &huart2;

void Print_Thread_Add()
    {

    UART_Mutex_Handle = xSemaphoreCreateMutexStatic(&UART_Mutex_Buffer);

    Print_Task_Handle = xTaskCreateStatic(Print_Task,
	    "DS18B20_Task",
	    PRINT_TASK_STACK_SIZE,
	    NULL,
	    PRINT_TASK_PRIORITY,
	    Print_Task_Stack,
	    &Print_Task_TCB);

    }

void CLI_UART_Send_Char(const char data)
    {

    CLI_UART->Instance->DR = (data);
    while (__HAL_UART_GET_FLAG(CLI_UART,UART_FLAG_TC) == 0);

    }

void CLI_UART_Send_String(const char* data)
    {

    /*** gaurd uart ***/
    xSemaphoreTake(UART_Mutex_Handle, portMAX_DELAY);

    uint16_t count = 0;
    while (*data)
	{
	CLI_UART_Send_Char(*data++);
	count++;
	}

    /*** release uart ***/
    xSemaphoreGive(UART_Mutex_Handle);
    }

void CLI_UART_Send_String_DMA(const char* data)
    {

    /*** gaurd uart ***/
    xSemaphoreTake(UART_Mutex_Handle, portMAX_DELAY);

    HAL_UART_Transmit_DMA(CLI_UART, (uint8_t*) data, strlen(data));

    /*** release uart ***/
    xSemaphoreGive(UART_Mutex_Handle);

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

    uint16_t q_data = 0;
    float temperature = 0.0;

    while (1)
	{

	//check if valid temperature is received from ds18b20 task within one second
	if (xQueueReceive(DS18B20_Q, &(q_data), (TickType_t) 1000))
	    {

	    temperature = (float) q_data / (float) 16;
	    CLI_UART_Send_Float(temperature);
	    CLI_UART_Send_String("\r\n");

	    }
	else
	    {
	    // Temperature sensor error
	    }

	}

    }
