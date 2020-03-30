/*
 * spi2.h
 *
 *  Created on: 30 mars 2020
 *      Author: pierre
 */

#ifndef SPI2_H_
#define SPI2_H_


#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/cm3/nvic.h>


void init_spi2(void);
uint8_t spi2_write(uint32_t  spi,uint16_t  	data );
void spi2_task(void *args);

#endif /* SPI2_H_ */
