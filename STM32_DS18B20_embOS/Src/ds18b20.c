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
#include "RTOS.h"


extern UART_HandleTypeDef huart6;
#define DS18B20_HALF_DUPLEX_UART huart6

/****ds18b20 thread data*/

#define          DS18B20_TASK_STACK_SIZE 256u
#define          DS18B20_TASK_PRIORITY   5u
OS_STACKPTR int  DS18B20_Task_Stack[DS18B20_TASK_STACK_SIZE];
OS_TASK          DS18B20_Task_TCB;
static void      DS18B20_Task();

/* only if accessing one wire port from more than one task */

OS_SEMAPHORE     DS18B20_SYN_SEM;
OS_MUTEX         DS18B20_Mutex;
OS_QUEUE         DS18B20_Q;

#define QUEUE_LENGTH    100
#define ITEM_SIZE       sizeof( uint16_t )

uint8_t          DS18B20_Q_Storage[OS_Q_SIZEOF_HEADER + QUEUE_LENGTH*ITEM_SIZE];

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

static uint8_t DS18B20_Scrach_Pad[9];

/*Maxim APPLICATION NOTE 27 */

static const uint8_t DS18B20_CRC_Table[] =
	{
	0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
	157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
	35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
	190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
	70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
	219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
	101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
	248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
	140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
	17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
	175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
	50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
	202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
	87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
	233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
	116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53
	};

uint8_t DS18B20_CRC(uint8_t* data, uint8_t len)
    {

    uint8_t crc = 0;

    for(uint8_t i=0; i<len; i++)
	{
	crc = DS18B20_CRC_Table[crc^ data[i]];
	}
    return crc;
    }


/**
 * @brief   Temperature convert, {Skip ROM = 0xCC, Convert = 0x44}
 */
static const uint8_t Temp_Convert_CMD[] =
    {
    BIT_0, BIT_0, BIT_1, BIT_1, BIT_0, BIT_0, BIT_1, BIT_1,
    BIT_0, BIT_0, BIT_1, BIT_0, BIT_0, BIT_0, BIT_1, BIT_0
    };

/**
 * @brief   Scratch data read, {Skip ROM = 0xCC, Scratch read = 0xBE}
 */
static const uint8_t TX_DMA_Buffer[] =
    {
    BIT_0, BIT_0, BIT_1, BIT_1, BIT_0, BIT_0, BIT_1, BIT_1, // Skip ROM = 0xCC
    BIT_0, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_0, BIT_1, // Scratch read = 0xBE

    BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, // Temperature LSB
    BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, // Temperature MSB

    BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, // TH
    BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, // TL

    BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, // CFG

    BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, // Reserved
    BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, // Reserved
    BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, // Reserved

    BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1, BIT_1  // CRC
    };

/**
 * @brief   Received temperature data using DMA
 */
static uint8_t RX_DMA_Buffer[sizeof(TX_DMA_Buffer)];


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

    OS_QUEUE_Create(&DS18B20_Q, DS18B20_Q_Storage, sizeof(DS18B20_Q_Storage));

    OS_MUTEX_Create(&DS18B20_Mutex);

    OS_SEMAPHORE_Create(&DS18B20_SYN_SEM, 0);

    OS_TASK_CREATE(&DS18B20_Task_TCB, "DS18B20_Task", 100, DS18B20_Task, DS18B20_Task_Stack);
    }

static void DS18B20_Task()
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
	    OS_Delay(750);

	    /* Send reset pulse */
	    DS18B20_Send_Reset();

	    /* Enable data reception with DMA */
	    DS18B20_Receive_Buffer(RX_DMA_Buffer, sizeof(RX_DMA_Buffer));

	    /* Send scratch pad data read command */
	    DS18B20_Send_Buffer(TX_DMA_Buffer, sizeof(TX_DMA_Buffer));

	    /* Wait until DMA receive scratch pad data */
	    OS_SEMAPHORE_TakeBlocked(&DS18B20_SYN_SEM);

	    /* Assemble DS18B20_Scrach Pad from  RX_DMA_Buffer */
	    for (uint8_t i = 0; i < 9; i++)
		{

		uint8_t temp = 0;

		for (uint8_t j = 0; j < 8; j++)
		    {

		    temp = temp >> 1;

		    if (BIT_1 == RX_DMA_Buffer[j + 16 + (i * 8)])
			{
			/* Bit value is 1 */
			temp |= 0x80;
			}

		    }

		DS18B20_Scrach_Pad[i] = temp;

		}

	    /* calculate crc */
	    uint8_t crc = DS18B20_CRC(DS18B20_Scrach_Pad, 8);

	    /* if crc matched data is valid*/
	    if (crc == DS18B20_Scrach_Pad[8])
		{
		/* Temporarily variable for extracting temperature data */
		uint16_t temperature = 0;

		temperature = DS18B20_Scrach_Pad[0]
			| DS18B20_Scrach_Pad[1] << 8;

		/* Send temperature. */

		OS_QUEUE_Put(&DS18B20_Q, &temperature, sizeof(temperature));

		/* Copying new temperature data and divide by 16 for fraction part */
		Current_Temperature = (float) temperature / (float) 16;
		}

	    }
	else
	    {
	    /* Temperature data not valid */
	    Current_Temperature = 0;
	    OS_Delay(10);
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
	OS_EnterNestableInterrupt();
	OS_SEMAPHORE_Give(&DS18B20_SYN_SEM);
        OS_LeaveNestableInterrupt();
	}
    }

