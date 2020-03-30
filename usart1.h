
#ifndef USART1_H

#define USART1_H

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include "miniprintf.h"
#include <string.h>
#include <stdlib.h>
#include "semphr.h"
#include "vesc/vesc.h"

void usart1_putc(char ch);
int usart1_printf(const char *format,...) __attribute((format(printf,1,2)));
void usart1_setup(void);
void usart1_task(void *args);
void usart1_printDouble( double val, unsigned int precision);
#endif


