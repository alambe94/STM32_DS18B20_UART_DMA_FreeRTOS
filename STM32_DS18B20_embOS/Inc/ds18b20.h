/*
 * ds18b20.h
 *
 *  Created on: May 28, 2019
 *      Author: medprime
 */

/*******************************************************************************
 * @file    DS18B20.h
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

#ifndef DS18B20_H_
#define DS18B20_H_

/* Includes */
#include "stm32f4xx_hal.h"

void DS18B20_GPIO_Init(void);
void DS18B20_UART_Init(void);
void DS18B20_TX_DMA_Init(void);
void DS18B20_RX_DMA_Init(void);
void DS18B20_Thread_Add();


#endif /* DS18B20_H_ */
