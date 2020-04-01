
#include "nrf24.h"

/*****************************************************SPI LINK SECTION : LINK THIS TO YOUR SPI + CE pin INTERFACE**********/

uint8_t spi_transfer(uint8_t tx)
{
    return (uint8_t)spi_xfer(SPI2,tx);
}



void set_ce(uint8_t state) // assume ce is connected to pb1
{
	if(state) gpio_set(GPIOB,GPIO1); else gpio_clear(GPIOB,GPIO1);

}
/* ------------------------------------------------------------------------- */
void set_csn(uint8_t state) // assume csn is connected to pb12
{
	if(state) gpio_set(GPIOB,GPIO12); else gpio_clear(GPIOB,GPIO12);
    }
/* ------------------------------------------------------------------------- */

/*****************************************************SPI LINK SECTION END**************************************************/


/* send and receive multiple bytes over SPI */
void nrf24_transferSync(uint8_t* dataout,uint8_t* datain,uint8_t len)
{
    uint8_t i;

    for(i=0;i<len;i++)
    {
        datain[i] = spi_transfer(dataout[i]);
    }

}

/* send multiple bytes over SPI */
void nrf24_transmitSync(uint8_t* dataout,uint8_t len)
{
    uint8_t i;
    
    for(i=0;i<len;i++)
    {
        spi_transfer(dataout[i]);
    }

}

/* Clocks only one byte into the given nrf24 register */
void nrf24_configRegister(uint8_t reg, uint8_t value)
{
    set_csn(0);
    spi_transfer(W_REGISTER | (REGISTER_MASK & reg));
    spi_transfer(value);
    set_csn(1);
}

/* Read single register from nrf24 */
void nrf24_readRegister(uint8_t reg, uint8_t* value, uint8_t len)
{
    set_csn(0);
    spi_transfer(R_REGISTER | (REGISTER_MASK & reg));
    nrf24_transferSync(value,value,len);
    set_csn(1);
}

/* Write to a single register of nrf24 */
void nrf24_writeRegister(uint8_t reg, uint8_t* value, uint8_t len)
{
    set_csn(0);
    spi_transfer(W_REGISTER | (REGISTER_MASK & reg));
    nrf24_transmitSync(value,len);
    set_csn(1);
}

void nrf24_config(uint8_t channel, uint8_t pay_length)
{
  

    // Set RF channel
    nrf24_configRegister(RF_CH,channel);

    // Set length of incoming payload 
    nrf24_configRegister(RX_PW_P0, 0x00); // Auto-ACK pipe ...
    nrf24_configRegister(RX_PW_P1, 0x00); // Data payload pipe
    nrf24_configRegister(RX_PW_P2, 0x00); // Pipe not used
    nrf24_configRegister(RX_PW_P3, 0x00); // Pipe not used
    nrf24_configRegister(RX_PW_P4, 0x00); // Pipe not used
    nrf24_configRegister(RX_PW_P5, 0x00); // Pipe not used



    // 1 Mbps, TX gain: 0dbm
    //nrf24_configRegister(RF_SETUP, (0<<RF_DR)|((0x03)<<RF_PWR));

	// 250Kbps, TXgain 0dbm 
	nrf24_configRegister(RF_SETUP,0b00100110);
	

    // CRC enable, 1 byte CRC length
    nrf24_configRegister(CONFIG,((1<<EN_CRC)|(0<<CRCO)));

    // Auto Acknowledgment
    nrf24_configRegister(EN_AA,(1<<ENAA_P0)|(1<<ENAA_P1)|(0<<ENAA_P2)|(0<<ENAA_P3)|(0<<ENAA_P4)|(0<<ENAA_P5));

    // Enable RX addresses
    nrf24_configRegister(EN_RXADDR,(1<<ERX_P0)|(1<<ERX_P1)|(0<<ERX_P2)|(0<<ERX_P3)|(0<<ERX_P4)|(0<<ERX_P5));

    // Auto retransmit delay: 1000 us and Up to 15 retransmit trials
    nrf24_configRegister(SETUP_RETR,(uint8_t)((0xFF<<ARD)|(0x0F<<ARC)));

   /* // Dynamic length configurations: No dynamic length
    nrf24_configRegister(DYNPD,(0<<DPL_P0)|(0<<DPL_P1)|(0<<DPL_P2)|(0<<DPL_P3)|(0<<DPL_P4)|(0<<DPL_P5));*/

    nrf24_enableDynamicPayload();

    // Start listening
    nrf24_powerUpRx();
}

void nrf24_powerUpRx(void)
{     
    set_csn(0);
    spi_transfer(FLUSH_RX);
    set_csn(1);

    nrf24_configRegister(STATUS,(1<<RX_DR)|(1<<TX_DS)|(1<<MAX_RT));

    set_ce(0);
    nrf24_configRegister(CONFIG,((1<<EN_CRC)|(0<<CRCO))|((1<<PWR_UP)|(1<<PRIM_RX)));
    set_ce(1);
}

void nrf24_rx_address(uint8_t * adr)
{
    set_ce(0);
    nrf24_writeRegister(RX_ADDR_P1,adr,nrf24_ADDR_LEN);
    set_ce(1);
}

void nrf24_tx_address(uint8_t* adr)
{
  
    nrf24_writeRegister(TX_ADDR,adr,nrf24_ADDR_LEN);
}

uint8_t nrf24_dataReady(void)
{
    // See note in getData() function - just checking RX_DR isn't good enough
    uint8_t status = nrf24_getStatus();

    // We can short circuit on RX_DR, but if it's not set, we still need
    // to check the FIFO for any pending packets
    if ( status & (1 << RX_DR) ) 
    {
        return 1;
    }

    return !nrf24_rxFifoEmpty();
}

/* Checks if receive FIFO is empty or not */
uint8_t nrf24_rxFifoEmpty(void)
{
    uint8_t fifoStatus;

    nrf24_readRegister(FIFO_STATUS,&fifoStatus,1);
    
    return (fifoStatus & (1 << RX_EMPTY));
}

/* Returns the length of data waiting in the RX fifo */
uint8_t nrf24_payloadLength(void)
{
    uint8_t status;
    set_csn(0);
    spi_transfer(R_RX_PL_WID);
    status = spi_transfer(0x00);
    set_csn(1);
    return status;
}


void nrf24_getData(uint8_t* dta)
{
    /* Pull down chip select */
    set_csn(0);

    /* Send cmd to read rx payload */
    spi_transfer( R_RX_PAYLOAD );
    
    /* Read payload */
    nrf24_transferSync(dta,dta,PAYLOAD_LEN);
    
    /* Pull up chip select */
    set_csn(1);

    /* Reset status register */
    nrf24_configRegister(STATUS,(1<<RX_DR));
}

/* Returns the number of retransmissions occured for the last message */
uint8_t nrf24_retransmissionCount(void)
{
    uint8_t rv;
    nrf24_readRegister(OBSERVE_TX,&rv,1);
    rv = rv & 0x0F;
    return rv;
}

// Sends a data package to the default address. Be sure to send the correct
// amount of bytes as configured as payload on the receiver.
void nrf24_send(uint8_t* value)
{    
    /* Go to Standby-I first */
    set_ce(0);
     
    /* Set to transmitter mode , Power up if needed */
    nrf24_powerUpTx();

    /* Do we really need to flush TX fifo each time ? */
    #if 1
        /* Pull down chip select */
        set_csn(0);

        /* Write cmd to flush transmit FIFO */
        spi_transfer(FLUSH_TX);     

        /* Pull up chip select */
        set_csn(1);
    #endif 

    /* Pull down chip select */
    set_csn(0);

    /* Write cmd to write payload */
    spi_transfer(W_TX_PAYLOAD);

    /* Write payload */
    nrf24_transmitSync(value,PAYLOAD_LEN);   

    /* Pull up chip select */
    set_csn(1);

    /* Start the transmission */
    set_ce(1);
}

uint8_t nrf24_isSending(void)
{
    uint8_t status;

    /* read the current status */
    status = nrf24_getStatus();
                
    /* if sending successful (TX_DS) or max retries exceded (MAX_RT). */
    if((status & ((1 << TX_DS)  | (1 << MAX_RT))))
    {        
        return 0; /* false */
    }

    return 1; /* true */

}

uint8_t nrf24_getStatus(void)
{
    uint8_t rv;
    set_csn(0);
    rv = spi_transfer(NOP);
    set_csn(1);
    return rv;
}

uint8_t nrf24_lastMessageStatus(void)
{
    uint8_t rv;

    rv = nrf24_getStatus();

    /* Transmission went OK */
    if((rv & ((1 << TX_DS))))
    {
        return NRF24_TRANSMISSON_OK;
    }
    /* Maximum retransmission count is reached */
    /* Last message probably went missing ... */
    else if((rv & ((1 << MAX_RT))))
    {
        return NRF24_MESSAGE_LOST;
    }  
    /* Probably still sending ... */
    else
    {
        return 0xFF;
    }
}

void nrf24_powerUpTx(void)
{
    nrf24_configRegister(STATUS,(1<<RX_DR)|(1<<TX_DS)|(1<<MAX_RT));

    nrf24_configRegister(CONFIG,nrf24_CONFIG|((1<<PWR_UP)|(0<<PRIM_RX)));
}

void nrf24_powerDown(void)
{
    set_ce(0);
    nrf24_configRegister(CONFIG,nrf24_CONFIG);
}


void nrf24_enableDynamicPayload(void)
{
	nrf24_configRegister(FEATURE,(1<<EN_DPL)); //enable dynamic payload feature

	nrf24_configRegister(DYNPD,(0x03)); //enable dynamic payload on P0 & P1
}

uint8_t getAvailablePaySize(void)
{
	return  spi_transfer(R_RX_PL_WID);
}



/*****************************DEBUG SECTION LINK THIS TO YOUR UART FUNCTION***************************************************/
// quite dirty but quick way to print register
void nrf24_displayConfiguration(void)

{

               uint8_t tab[5] = { 0U };

               nrf24_readRegister(CONFIG,tab,1);

               usart1_printf("CONFIG REGISTER(Configuration Register) :  ");

               printTable(tab,1);

               usart1_printf("\r\n");

               clearTable(tab,5);

              

               nrf24_readRegister(EN_AA,tab,1);

               usart1_printf("EN_AA REGISTER (Enable Auto Acknowledgment):  ");

               printTable(tab,1);

               usart1_printf("\r\n");

               clearTable(tab,5);

              

               nrf24_readRegister(EN_RXADDR,tab,1);

               usart1_printf("EN_RXADDR (Enabled RX pipes):  ");

               printTable(tab,1);

               usart1_printf("\r\n");

               clearTable(tab,5);

              

               nrf24_readRegister(SETUP_AW,tab,1);

               usart1_printf("SETUP_AW (Setup of Address Width)");

               usart1_printf(" (common for all data pipes) : ");

               printTable(tab,1);

               usart1_printf("\r\n");

               clearTable(tab,5);

              

               nrf24_readRegister(SETUP_RETR,tab,1);

               usart1_printf("SETUP_RETR (Setup of Automatic Retransmission):  ");

               printTable(tab,1);

               usart1_printf("\r\n");

               clearTable(tab,5);

              

               nrf24_readRegister(RF_CH,tab,1);

               usart1_printf("RF_CH (RF Channel):  ");

               printTable(tab,1);

               usart1_printf("\r\n");

               clearTable(tab,5);

              

               nrf24_readRegister(RF_SETUP,tab,1);

               usart1_printf("RF_SETUP (RF Setup Register):  ");

               printTable(tab,1);

               usart1_printf("\r\n");

               clearTable(tab,5);

              

               nrf24_readRegister(STATUS,tab,1);

               usart1_printf("STATUS (Status Register):  ");

               printTable(tab,1);

               usart1_printf("\r\n");

               clearTable(tab,5);

              

               nrf24_readRegister(RX_ADDR_P0,tab,5);

               usart1_printf("RX_ADDR_P0 (Receive address data pipe 0):");

               printTable(tab,5);

               usart1_printf("\r\n");

               clearTable(tab,5);

              

               nrf24_readRegister(RX_ADDR_P1,tab,5);

               usart1_printf("RX_ADDR_P1 (Receive address data pipe 1):");

               printTable(tab,5);

               usart1_printf("\r\n");

               clearTable(tab,5);

              

               nrf24_readRegister(RX_ADDR_P2,tab,5);

               usart1_printf("RX_ADDR_P2 (Receive address data pipe 2):");

               printTable(tab,5);

               usart1_printf("\r\n");

               clearTable(tab,5);

              

               nrf24_readRegister(RX_ADDR_P3,tab,5);

               usart1_printf("RX_ADDR_P3 (Receive address data pipe 3):");

               printTable(tab,5);

               usart1_printf("\r\n");

               clearTable(tab,5);

              

               nrf24_readRegister(RX_ADDR_P4,tab,5);

               usart1_printf("RX_ADDR_P4 (Receive address data pipe 4):");

               printTable(tab,5);

               usart1_printf("\r\n");

               clearTable(tab,5);

              

               nrf24_readRegister(RX_ADDR_P5,tab,5);

               usart1_printf("RX_ADDR_P5 (Receive address data pipe 5):");

               printTable(tab,5);

               usart1_printf("\r\n");

               clearTable(tab,5);

              

               nrf24_readRegister(TX_ADDR,tab,5);

               usart1_printf("TX_ADDR (Transmit address):");

               printTable(tab,5);

               usart1_printf("\r\n");

               clearTable(tab,5);

              

              

               nrf24_readRegister(RX_PW_P0,tab,1);

               usart1_printf("RX_PW_P0 (Number of bytes in RX payload in data pipe 0):");

               printTable(tab,1);

               usart1_printf("\r\n");

               clearTable(tab,5);

              

               nrf24_readRegister(RX_PW_P1,tab,1);

               usart1_printf("RX_PW_P1 (Number of bytes in RX payload in data pipe 1):");

               printTable(tab,1);

               usart1_printf("\r\n");

               clearTable(tab,5);

              

               nrf24_readRegister(RX_PW_P2,tab,1);

               usart1_printf("RX_PW_P2 (Number of bytes in RX payload in data pipe 2):");

               printTable(tab,1);

               usart1_printf("\r\n");

               clearTable(tab,5);

              

               nrf24_readRegister(RX_PW_P3,tab,1);

               usart1_printf("RX_PW_P3 (Number of bytes in RX payload in data pipe 3):");

               printTable(tab,1);

               usart1_printf("\r\n");

               clearTable(tab,5);

              

               nrf24_readRegister(RX_PW_P4,tab,1);

               usart1_printf("RX_PW_P4 (Number of bytes in RX payload in data pipe 4):");

               printTable(tab,1);

               usart1_printf("\r\n");

               clearTable(tab,5);

              

               nrf24_readRegister(RX_PW_P5,tab,1);

               usart1_printf("RX_PW_P5 (Number of bytes in RX payload in data pipe 5):");

               printTable(tab,1);

               usart1_printf("\r\n");

               clearTable(tab,5);

              

               nrf24_readRegister(DYNPD,tab,1);

               usart1_printf("DYNPD (Enable dynamic payload length):");

               printTable(tab,1);

               usart1_printf("\r\n");

               clearTable(tab,5);

              

             nrf24_readRegister(FEATURE,tab,1);

              usart1_printf("FEATURE  (Feature Register):");

              printTable(tab,1);

              usart1_printf("\r\n");

              clearTable(tab,5);

               
               
              

              

} 

void clearTable(uint8_t  *tab,uint8_t size)
{
    uint8_t i ;
    for(i = 0; i<size;i++)
    {
        tab[i] = 0U ;
    }
}

void printTable(uint8_t  *tab,uint8_t size)
{
	 uint8_t i ;
	for(i = 0; i<size;i++)
	    {
		usart1_printf("0x%02x ",tab[i]);
	    }

}



