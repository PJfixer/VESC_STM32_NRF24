

#include "usart1.h"

//extern TaskHandle_t h_task_usart1;

size_t lenPayload_usart1; // bye in the usart buffer
uint8_t payload_usart1[10]; // usart buffer fed by uart isr

extern SemaphoreHandle_t xSemaphore_usart1;

//extern nunchuckPackage nunchuck;

inline void
usart1_putc(char ch) {
	usart_send_blocking(USART1,ch);
}

int
usart1_printf(const char *format,...) {

				va_list args;
				int rc=0;

				va_start(args,format);
				if( xSemaphoreTake( xSemaphore_usart1, ( TickType_t ) 10 ) == pdTRUE )  //CHECK IF USART1 IS available
					{


						rc = mini_vprintf_cooked(usart1_putc,format,args);
						xSemaphoreGive( xSemaphore_usart1 );

					}
				va_end(args);
				return rc;


}
/*********************************************************************
 * Setup the UART
 *********************************************************************/
void usart1_setup(void) {

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_USART1);
	rcc_periph_clock_enable(RCC_AFIO);

	//usart_enable_rx_interrupt(USART1);

	/* Enable the USART1 interrupt. */
	//nvic_enable_irq(NVIC_USART1_IRQ);



	// UART TX on PA9 (GPIO_USART1_TX)
	gpio_set_mode(GPIOA,GPIO_MODE_OUTPUT_50_MHZ,GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,GPIO_USART1_TX);
	// UART RX on PA10 (GPIO_USART1_RX)
	gpio_set_mode(GPIOA,GPIO_MODE_INPUT,GPIO_CNF_INPUT_FLOAT,GPIO_USART1_RX);

	usart_set_baudrate(USART1,115200);
	usart_set_databits(USART1,8);
	usart_set_stopbits(USART1,USART_STOPBITS_1);
	usart_set_mode(USART1,USART_MODE_TX_RX);
	usart_set_parity(USART1,USART_PARITY_NONE);
	usart_set_flow_control(USART1,USART_FLOWCONTROL_NONE);

	usart_enable(USART1);

}


/*void usart1_isr(void)
{
	BaseType_t woken = pdFALSE;
	//gpio_toggle(GPIOC,GPIO13);


	//Check if we were called because of RXNE.
	if (((USART_CR1(USART1) & USART_CR1_RXNEIE) != 0) &&
	    ((USART_SR(USART1) & USART_SR_RXNE) != 0)) {




		uint8_t byte = usart_recv_blocking(USART1);
		payload_usart1[lenPayload_usart1] = byte;
		lenPayload_usart1++;
		if(byte == 13) // cariage return
		{
			vTaskNotifyGiveFromISR(h_task_usart1,&woken);

		}
		if(lenPayload_usart1 >10)
		{
			lenPayload_usart1=0;
		}
		portYIELD_FROM_ISR(woken);


	}

}*/

void usart1_printDouble( double val, unsigned int precision){

	usart1_printf("%d",(int)val);  //prints the int part
	usart1_putc('.'); // print the decimal point
    unsigned int frac;
    if(val >= 0)
    {
	  frac = (int) ((val - (int)val) * precision);
    }
    else{
    	frac =(int) (((int)val- val)  * precision);

    }
    usart1_printf("%d",frac) ;
}


/*void usart1_task(void *args) {
	(void)args;
	 int16_t parsed_int = 0;
	 uint8_t temp[10];
		for(;;)
		{
			// Block execution until notified
			ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
			switch(payload_usart1[0]) // look for the commannd type
			{
			case 'F':  //case its a foward command
				memcpy(temp,&payload_usart1[1],lenPayload_usart1-2);
				parsed_int = atoi((const char*)temp);
				//nunchuck.valueY = parsed_int;
				//setNunchuckValues();
				VescUartSetRPM((int16_t)parsed_int);
				usart1_printf("%d \n",parsed_int);
			break;
			case 'R':  //case its a reverse command
				memcpy(temp,&payload_usart1[1],lenPayload_usart1-2);
				parsed_int = atoi((const char*)temp);
				VescUartSetRPMfwd((int16_t)parsed_int,90);
				usart1_printf("%d \n",parsed_int);
			break;
			default:
					//default command
			break;


			}
			lenPayload_usart1 = 0;
			//parse data & send

		}
}*/

