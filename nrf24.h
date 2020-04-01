


#include "nRF24L01.h"
#include "usart1.h"
#include "spi2.h"

#define nrf24_ADDR_LEN 5
#define nrf24_CONFIG ((1<<EN_CRC)|(0<<CRCO))

#define NRF24_TRANSMISSON_OK 0
#define NRF24_MESSAGE_LOST   1

#define PAYLOAD_LEN 4



void set_ce(uint8_t state);
void set_csn(uint8_t state);

uint8_t spi_transfer(uint8_t tx);

void nrf24_transferSync(uint8_t* dataout,uint8_t* datain,uint8_t len);
void nrf24_transmitSync(uint8_t* dataout,uint8_t len);
void nrf24_configRegister(uint8_t reg, uint8_t value);
void nrf24_readRegister(uint8_t reg, uint8_t* value, uint8_t len);
void nrf24_writeRegister(uint8_t reg, uint8_t* value, uint8_t len) ;
void nrf24_powerUpRx(void);
void nrf24_config(uint8_t channel, uint8_t pay_length);
void nrf24_displayConfiguration(void);
void clearTable(uint8_t* tab,uint8_t size);
void printTable(uint8_t  *tab,uint8_t size);
void nrf24_rx_address(uint8_t * adr) ;
void nrf24_tx_address(uint8_t* adr);
uint8_t nrf24_dataReady(void) ;
uint8_t nrf24_rxFifoEmpty(void);
uint8_t nrf24_payloadLength(void);
void nrf24_getData(uint8_t* dta) ;
uint8_t nrf24_retransmissionCount(void);
void nrf24_send(uint8_t* value);
uint8_t nrf24_isSending(void);
uint8_t nrf24_getStatus(void);
uint8_t nrf24_lastMessageStatus(void);
void nrf24_powerUpTx(void);
void nrf24_powerDown(void);
void nrf24_enableDynamicPayload(void);
uint8_t getAvailablePaySize(void);

