

#include "usart3.h"
#include "usart1.h"
#include "vesc/vesc.h"

size_t lenPayload_usart3; // bye in the usart buffer
uint8_t payload_usart3[100]; // usart buffer fed by uart isr

extern SemaphoreHandle_t xSemaphore_usart3;

extern dataPackage data_vesc1; // vesc measurement structure data
extern dataPackage data_vesc2; // vesc measurement structure data
extern Custom_dataPackage Telemetry_data; //vesc

inline void
usart3_putc(char ch) {
	usart_send_blocking(USART3,ch);
}

int
usart3_printf(const char *format,...) {

	va_list args;
	int rc=0;

	va_start(args,format);
	if( xSemaphoreTake( xSemaphore_usart3, 0 ) == pdTRUE )  //CHECK IF USART3 IS available
		{
			rc = mini_vprintf_cooked(usart3_putc,format,args);
			xSemaphoreGive( xSemaphore_usart3 );
		}
	va_end(args);
	return rc;
}
/*********************************************************************
 * Setup the UART
 *********************************************************************/
void usart3_setup(void) {

	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_USART3);
	rcc_periph_clock_enable(RCC_AFIO);

	usart_enable_rx_interrupt(USART3);

	/* Enable the usart3 interrupt. */
	nvic_enable_irq(NVIC_USART3_IRQ);



	// UART TX on PB10 (GPIO_usart3_TX)
	gpio_set_mode(GPIOB,GPIO_MODE_OUTPUT_50_MHZ,GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,GPIO_USART3_TX);
	// UART RX on PB11 (GPIO_usart3_RX)
	gpio_set_mode(GPIOB,GPIO_MODE_INPUT,GPIO_CNF_INPUT_FLOAT,GPIO_USART3_RX);

	usart_set_baudrate(USART3,115200);
	usart_set_databits(USART3,8);
	usart_set_stopbits(USART3,USART_STOPBITS_1);
	usart_set_mode(USART3,USART_MODE_TX_RX);
	usart_set_parity(USART3,USART_PARITY_NONE);
	usart_set_flow_control(USART3,USART_FLOWCONTROL_NONE);

	usart_enable(USART3);

}


void usart3_isr(void)
{
	BaseType_t woken = pdFALSE;
	//gpio_toggle(GPIOC,GPIO13);


	/* Check if we were called because of RXNE. */
	if (((USART_CR1(USART3) & USART_CR1_RXNEIE) != 0) &&
	    ((USART_SR(USART3) & USART_SR_RXNE) != 0)) {




		uint8_t byte = usart_recv_blocking(USART3);
		payload_usart3[lenPayload_usart3] = byte;
		lenPayload_usart3++;

		/*if(lenPayload_usart3 >= 100)
		{

			lenPayload_usart3 = 0;
		}*/


	}
	 portYIELD_FROM_ISR( &woken );
}

void usart3_task(void *args) {
	(void)args;
	lenPayload_usart3 = 0;

		for(;;)
		{
			taskENTER_CRITICAL();
			if(getVescValues() == true)
			{
				usart1_printf("success read on vesc1 \n");
				usart1_printf("data.tacho : %d \n ",(int)data_vesc1.tachometer);
				/*usart1_printf("Vesc1 : avgMotorCurrent : "); usart1_printDouble(data.avgMotorCurrent,100);usart1_putc('\n');
				usart1_printf("Vesc1 : avgMotorCurrent : "); usart1_printDouble(data.avgMotorCurrent,100);usart1_putc('\n');
				usart1_printf("Vesc1 : avgMotorCurrent : "); usart1_printDouble(data.avgMotorCurrent,100);usart1_putc('\n');
				usart1_printf("Vesc1 : avgMotorCurrent : "); usart1_printDouble(data.avgMotorCurrent,100);usart1_putc('\n');
				usart1_printf("Vesc1 : avgMotorCurrent : "); usart1_printDouble(data.avgMotorCurrent,100);usart1_putc('\n');*/
			}
			else
			{
				usart1_printf("failed read on vesc1 \n");


			}


			if(getVescValuesfwd(90) == true)
			{
				usart1_printf("success read on vesc2 \n");
				usart1_printf("data.tacho : %d \n ",(int)data_vesc2.tachometer);
			}
			else
			{
				usart1_printf("failed read on vesc2 \n");
			}
			taskEXIT_CRITICAL();
			Telemetry_data.Dual_ampHours  = (data_vesc1.ampHours + data_vesc2.ampHours);
			Telemetry_data.Dual_ampHoursCharged = (data_vesc1.ampHoursCharged + data_vesc2.ampHoursCharged);
			Telemetry_data.Dual_inpVoltage = data_vesc1.inpVoltage;
			Telemetry_data.Mot1_avgMotorCurrent = data_vesc1.avgMotorCurrent;
			Telemetry_data.Mot2_avgMotorCurrent = data_vesc2.avgMotorCurrent;
			Telemetry_data.Mot1_rpm = data_vesc1.rpm ;
			Telemetry_data.Mot1_tachometerAbs = data_vesc1.tachometerAbs;
			vTaskDelay(pdMS_TO_TICKS(10000));
		}
}

