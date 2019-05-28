/*
 * ds18b20.c
 *
 *  Created on: May 28, 2019
 *      Author: medprime
 */

/*******************************************************************************
 * @file    DS18B20.c
 * @author  Ahmed Eldeep
 * @email   ahmed@almohandes.org
 * @website http://almohandes.org/stm32
 * @date    22.05.2018
 *
 * @brief   Interfacing temperature sensor DS18B20 using UART over one-wire
 * @note
 *
 @verbatim
 Copyright (C) Almohandes.org, 2018

 This program is free software: you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free Software
 Foundation, either version 3 of the License, or any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with
 this program.  If not, see <http://www.gnu.org/licenses/>.
 @endverbatim
 *******************************************************************************/

/* Includes */
#include "ds18b20.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

extern UART_HandleTypeDef huart3;
#define DS18B20_HALF_DUPLEX_UART huart3

/****ds18b20 thread data*/

#define      DS18B20_TASK_STACK_SIZE 256u
#define      DS18B20_TASK_PRIORITY   7u
TaskHandle_t DS18B20_Task_Handle;
StackType_t  DS18B20_Task_Stack[DS18B20_TASK_STACK_SIZE];
StaticTask_t DS18B20_Task_TCB;
static void  DS18B20_Task(void* argument);

SemaphoreHandle_t DS18B20_Mutex_Handle;
StaticSemaphore_t DS18B20_Mutex_Buffer;

/****ds18b20 thread data*/


#define DS18B20_RESET_CMD                    ((uint8_t) 0xF0)

#define BIT_0                                ((uint8_t) 0x00)
#define BIT_1                                ((uint8_t) 0xFF)
#define MAX_CONVERSION_TIME                  ((uint32_t) 750)

#define DS18B20_READ_ROM              0x33  /* 0b00110011  */
#define DS18B20_SKIP_ROM              0xCC  /* 0b11001100  */
#define DS18B20_MATCH_ROM             0x55  /* 0b01010101  */
#define DS18B20_CONVERT_TEM           0x44  /* 0b01000100  */
#define DS18B20_READ_RAM              0xBE  /* 0b10111110  */
#define DS18B20_WRITE_RAM             0x4E  /* 0b01001110  */

/**
 * @brief   Temperature convert, {Skip ROM = 0xCC, Convert = 0x44}
 */
static const uint8_t Temp_Convert_CMD[] =
    {
    BIT_0, BIT_0, BIT_1, BIT_1, BIT_0, BIT_0, BIT_1, BIT_1,
    BIT_0, BIT_0, BIT_1, BIT_0, BIT_0, BIT_0, BIT_1, BIT_0
    };

/**
 * @brief   Temperature data read, {Skip ROM = 0xCC, Scratch read = 0xBE}
 */
static const uint8_t Temp_Read_CMD[] =
    {
    BIT_0, BIT_0, BIT_1, BIT_1, BIT_0, BIT_0, BIT_1, BIT_1,
    BIT_0, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_0, BIT_1,
    BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1,
    BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1
    };

/**
 * @brief   Received temperature data using DMA
 */
static uint8_t Temperature_Data[sizeof(Temp_Read_CMD)];

/**
 * @brief   Current temperature value in degree celsius
 */
static float Current_Temperature = 0;

/**
 * @brief   Configure GPIO
 * @note
 * @param   None
 * @retval  None
 */
void DS18B20_GPIO_Init(void)
    {
    /*
     * Configured in cube.
     * See usart.c
     */
    }

/**
 * @brief   Configure UART for DS18B20
 * @note
 * @param   None
 * @retval  None
 */

void DS18B20_UART_Init(void)
    {
    /*
     * Configured in cube.
     * See usart.c
     */
    }

/**
 * @brief   Configure DMA for UART TX
 * @note
 * @param   None
 * @retval  None
 */
void DS18B20_TX_DMA_Init(void)
    {
    /*
     * Configured in cube.
     * See usart.c
     */
    }

/**
 * @brief   Configure DMA for UART RX
 * @note
 * @param   None
 * @retval  None
 */
void DS18B20_RX_DMA_Init(void)
    {
    /*
     * Configured in cube.
     * See usart.c
     */
    }

static void DS18B20_Send_Buffer(const uint8_t * cmd, uint8_t size)
    {
    HAL_UART_Transmit_DMA(&DS18B20_HALF_DUPLEX_UART, (uint8_t*) cmd, size);
    }

static void DS18B20_Receive_Buffer(const uint8_t * cmd, uint8_t size)
    {
    HAL_UART_Receive_DMA(&DS18B20_HALF_DUPLEX_UART, (uint8_t*) cmd, size);
    }

static uint8_t DS18B20_Send_Reset(void)
    {
    uint8_t isSensorDetected = 0;

    /* Change baud rate to 9600 */
    DS18B20_HALF_DUPLEX_UART.Init.BaudRate = 9600;

    if (HAL_HalfDuplex_Init(&DS18B20_HALF_DUPLEX_UART) != HAL_OK)
	{
	Error_Handler();
	}

    /* Write reset command */
    uint8_t temp = DS18B20_RESET_CMD;
    HAL_UART_Transmit(&DS18B20_HALF_DUPLEX_UART, &temp, 1, 10);

    /* Read Rx Data */
    uint8_t Rx;
    HAL_UART_Receive(&DS18B20_HALF_DUPLEX_UART, &Rx, 1, 10);

    /* Check sensor presence */
    if ((DS18B20_RESET_CMD != Rx) && ( BIT_0 != Rx))
	{
	/* Temp sensor was detected */
	isSensorDetected = 1;
	}
    else
	{
	/* Do nothing, No sensor was detected */
	}

    /* Change baud rate to 115200 */
    DS18B20_HALF_DUPLEX_UART.Init.BaudRate = 115200;

    if (HAL_HalfDuplex_Init(&DS18B20_HALF_DUPLEX_UART) != HAL_OK)
	{
	Error_Handler();
	}

    return isSensorDetected;
    }

void DS18B20_Thread_Add()
    {

    DS18B20_Mutex_Handle = xSemaphoreCreateMutexStatic(&DS18B20_Mutex_Buffer);
    xSemaphoreGive(DS18B20_Mutex_Handle);

    DS18B20_Task_Handle = xTaskCreateStatic(DS18B20_Task,
	    "DS18B20_Task",
	    DS18B20_TASK_STACK_SIZE,
	    NULL,
	    DS18B20_TASK_PRIORITY,
	    DS18B20_Task_Stack,
	    &DS18B20_Task_TCB);
    }

static void DS18B20_Task(void* argument)
    {

    DS18B20_GPIO_Init();
    DS18B20_UART_Init();
    DS18B20_TX_DMA_Init();
    DS18B20_RX_DMA_Init();

    while (1)
	{
	/* Sensor detected flag */
	uint8_t isSensorDetected = 0;

	/* Send reset pulse */
	isSensorDetected = DS18B20_Send_Reset();

	/* Check if the sensor was detected */
	if (1 == isSensorDetected)
	    {

	    /* Send temperature conversion command */
	    DS18B20_Send_Buffer(Temp_Convert_CMD, sizeof(Temp_Convert_CMD));

	    /* Wait conversion time */
	    vTaskDelay(1000);

	    /* Send reset pulse */
	    DS18B20_Send_Reset();

	    /* Enable temperature data reception with DMA */
	    DS18B20_Receive_Buffer(Temperature_Data, sizeof(Temperature_Data));

	    /* Send temperature read command */
	    DS18B20_Send_Buffer(Temp_Read_CMD, sizeof(Temp_Read_CMD));

	    /* Wait until DMA receive temperature data */
	    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

	    /* Temporarily variable for extracting temperature data */
	    uint16_t temperature = 0;

	    /* Extract new temperature data */
	    for (int idx = 16; idx < 32; idx++)
		{
		if (BIT_1 == Temperature_Data[idx])
		    {
		    /* Bit value is 1 */
		    temperature = (temperature >> 1) | 0x8000;
		    }
		else
		    {
		    /* Bit value is 0 */
		    temperature = temperature >> 1;
		    }
		}

	    /* Copying new temperature data and divide by 16 for fraction part */
	    Current_Temperature = (float) temperature / (float) 16;

	    /**** notify other tasks **/
	    /**** send temperature via queue**/

	    }
	else
	    {
	    /* Temperature data not valid */
	    Current_Temperature = 0;
	    }
	}
    }

/**
 * @brief   IRQ callback function
 * @note
 * @param   None
 * @retval  None
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
    {

    }

/**
 * @brief   IRQ callback function
 * @note
 * @param   None
 * @retval  None
 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
    {

    if (huart == &DS18B20_HALF_DUPLEX_UART)
	{
	BaseType_t xHigherPriorityTaskWoken;
	vTaskNotifyGiveFromISR(DS18B20_Task_Handle, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
    }

