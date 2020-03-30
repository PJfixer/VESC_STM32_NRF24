/*
 * spi2.c
 *
 *  Created on: 30 mars 2020
 *      Author: pierre
 */


void init_spi2(void)
{
	//enable spi2 clock0
	rcc_periph_clock_enable(RCC_SPI1);
	// configure GPIO for spi2
		gpio_set_mode(GPIOA,GPIO_MODE_OUTPUT_50_MHZ,GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,PIO4|GPIO5|GPIO7);		// NSS=PA4,SCK=PA5,MOSI=PA7)
		gpio_set_mode(
			GPIOA,
			GPIO_MODE_INPUT,
			GPIO_CNF_INPUT_FLOAT,
			GPIO6				// MISO=PA6
		);
		spi_reset(SPI1);
		spi_init_master(
			SPI1,
	                SPI_CR1_BAUDRATE_FPCLK_DIV_256,
	                SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
			SPI_CR1_CPHA_CLK_TRANSITION_1,
		        SPI_CR1_DFF_8BIT,
		        SPI_CR1_MSBFIRST
		);
		spi_disable_software_slave_management(SPI1);
		spi_enable_ss_output(SPI1);


}
