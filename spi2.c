/*
 * spi2.c
 *
 *  Created on: 30 mars 2020
 *      Author: pierre
 */

#include "spi2.h"

void init_spi2(void)
{
	//enable spi2 clock0
	rcc_periph_clock_enable(RCC_SPI2);
	// configure GPIO for spi2
	gpio_set_mode(GPIOB,GPIO_MODE_OUTPUT_50_MHZ,GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,GPIO13|GPIO15);//SCK=PB13,MOSI=PB15)
	gpio_set_mode(GPIOB,GPIO_MODE_INPUT,GPIO_CNF_INPUT_FLOAT,GPIO14);				// MISO=PB14

	rcc_periph_clock_enable(RCC_GPIOB);
	gpio_set_mode(GPIOB,GPIO_MODE_OUTPUT_50_MHZ,GPIO_CNF_OUTPUT_PUSHPULL,GPIO12); //CS = PB12


		spi_reset(SPI2);
		spi_init_master(
			SPI2,
	        SPI_CR1_BAUDRATE_FPCLK_DIV_4,
	        SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
			SPI_CR1_CPHA_CLK_TRANSITION_1,
		    SPI_CR1_DFF_8BIT,
		    SPI_CR1_MSBFIRST
		);
		spi_disable_software_slave_management(SPI2);
		//spi_enable_ss_output(SPI2);
		spi_enable(SPI2);


}

uint8_t spi2_write(uint32_t  spi,uint16_t  	data )
{
	uint8_t reiceived_data ;
	gpio_clear(GPIOB,GPIO12); // put CS down (0)
	reiceived_data = (uint8_t)spi_xfer(spi,data);
	gpio_set(GPIOB,GPIO12); // put CS high (0)
	return reiceived_data;
}





