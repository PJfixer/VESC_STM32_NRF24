/* Simple LED task demo:
 *
 * The LED on PC13 is toggled in task1.
 */
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "usart1.h"
#include "usart3.h"
#include "semphr.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>

TaskHandle_t h_task_usart1=0;

SemaphoreHandle_t xSemaphore_usart1 = NULL;
SemaphoreHandle_t xSemaphore_usart3 = NULL;

static void
gpio_setup(void) {


	rcc_periph_clock_enable(RCC_GPIOC);
	gpio_set_mode(GPIOC,GPIO_MODE_OUTPUT_2_MHZ,GPIO_CNF_OUTPUT_PUSHPULL,GPIO13);
}


static void
task1(void *args) {
	int i;

	(void)args;

	for (;;) {
		gpio_toggle(GPIOC,GPIO13);

		for (i = 0; i < 3000000; i++)
			__asm__("nop");
	}
}

/*static void drive(void *args) {

	(void)args;
	 for (;;) {

	}
}*/



int
main(void) {

	rcc_clock_setup_in_hse_8mhz_out_72mhz();	// Use this for "blue pill"
	gpio_setup();
	usart1_setup();
	usart3_setup();

	xSemaphore_usart1 = xSemaphoreCreateMutex();
	xSemaphore_usart3 = xSemaphoreCreateMutex();
	xSemaphoreGive( xSemaphore_usart3 );
	xSemaphoreGive( xSemaphore_usart1 );


	xTaskCreate(task1,"LED",50,NULL,configMAX_PRIORITIES-3,NULL);
	xTaskCreate(usart1_task,"PC_COMMANDS",150,NULL,configMAX_PRIORITIES-1,&h_task_usart1);
	xTaskCreate(usart3_task,"VESC_MEASURE",150,NULL,configMAX_PRIORITIES-2,NULL);
	vTaskStartScheduler();
	for (;;)
		;
	return 0;
}

// End
