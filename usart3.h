
#ifndef usart3_H

#define usart3_H

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include "miniprintf.h"
#include "semphr.h"

void usart3_putc(char ch);
int usart3_printf(const char *format,...) __attribute((format(printf,1,2)));
void usart3_setup(void);
void usart3_task(void *args);

#endif


