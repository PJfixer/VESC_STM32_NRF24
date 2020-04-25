/* Simple LED task demo:
 *
 * The LED on PC13 is toggled in task1.
 */
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "usart1.h"
#include "usart3.h"
#include "spi2.h"
#include "nrf24.h"

#include "semphr.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>


TaskHandle_t h_nrf24_RX=0;

SemaphoreHandle_t xSemaphore_usart1 = NULL;
SemaphoreHandle_t xSemaphore_usart3 = NULL;

void exti0_isr(void)
{
	exti_reset_request(EXTI0);
	BaseType_t woken = pdFALSE;
	//usart1_printf("it \n");
	vTaskNotifyGiveFromISR(h_nrf24_RX,&woken);
}


static void
gpio_setup(void) {


	rcc_periph_clock_enable(RCC_GPIOC);
	gpio_set_mode(GPIOC,GPIO_MODE_OUTPUT_2_MHZ,GPIO_CNF_OUTPUT_PUSHPULL,GPIO13);


	//set GPIO for NRF24 radio
	//CE
	rcc_periph_clock_enable(RCC_GPIOB);
	gpio_set_mode(GPIOB,GPIO_MODE_OUTPUT_2_MHZ,GPIO_CNF_OUTPUT_PUSHPULL,GPIO1); // CE on PB1

	//CS is configured via SPI (spi2.c)


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
	init_spi2();


	xSemaphore_usart1 = xSemaphoreCreateMutex();
	xSemaphore_usart3 = xSemaphoreCreateMutex();
	xSemaphoreGive( xSemaphore_usart3 );
	xSemaphoreGive( xSemaphore_usart1 );


	uint8_t tx_address1[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};
	uint8_t rx_address1[5] = {0xD7,0xD5,0xD7,0xD7,0xD7};

	//uint8_t tx_address1[5] =  {0xD7,0xD5,0xD7,0xD7,0xD7};
	//uint8_t rx_address1[5] =  {0xE7,0xE7,0xE7,0xE7,0xE7};

	 nrf24_tx_address(tx_address1);
	 nrf24_rx_address(rx_address1);

	 //Wait before configure nrf24
	 	for (int i = 0; i < 3000000; i++)
	 				__asm__("nop");

	 //IMPORTANT if use auto ack put the TX address also in the P0_RX pipe to receive the ack
	 nrf24_writeRegister(RX_ADDR_P0,tx_address1,nrf24_ADDR_LEN); // set AUTO-ACK PIPE0 adrres = TX ADDRES


	 nrf24_config(20,PAYLOAD_LEN);


	 nrf24_enableOnRX_interupt();

	 nrf24_displayConfiguration();
	 //nrf24_powerDown();



	//configure RF24 interrupt

	 gpio_set_mode(GPIOA,			// nrf24 IRQ
	 		GPIO_MODE_INPUT,		// Input
	 		GPIO_CNF_INPUT_FLOAT,
	 		GPIO0);			// on PC14

	 	exti_select_source(EXTI0,GPIOA);
	 	exti_set_trigger(EXTI0,EXTI_TRIGGER_FALLING);
	 	exti_enable_request(EXTI0);
	 	nvic_enable_irq(NVIC_EXTI0_IRQ);	// PA0 <- IRQ



	xTaskCreate(nrf24_RX,"RF24_RX_test",150,NULL,(configMAX_PRIORITIES-1),&h_nrf24_RX);

	xTaskCreate(task1,"LED",50,NULL,(configMAX_PRIORITIES-3),NULL);
	//xTaskCreate(usart1_task,"PC_COMMANDS",150,NULL,configMAX_PRIORITIES-1,&h_task_usart1);
	xTaskCreate(usart3_task,"VESC_MEASURE",150,NULL,(configMAX_PRIORITIES-2),NULL);
	//xTaskCreate(nrf24_TX,"RF24_TX_test",100,NULL,configMAX_PRIORITIES-2,NULL);


	vTaskStartScheduler();
	for (;;)
	{

	}

	return 0;
}

// End
