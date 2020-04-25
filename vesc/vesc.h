
#ifndef VESC_H

#define VESC_H

#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include "usart3.h"
#include "buffer.h"
#include "crc.h"
#include "datatypes.h"

typedef struct  {
		float Mot1_avgMotorCurrent;
		float Mot2_avgMotorCurrent;
		float Dul_avgInputCurrent;
		long Mot1_rpm;
		float Dual_inpVoltage;
		float Dual_ampHours;
		float Dual_ampHoursCharged;
		long Mot1_tachometerAbs;
	}Custom_dataPackage;


/** Struct to store the telemetry data returned by the VESC */
	typedef struct  {
		float avgMotorCurrent;
		float avgInputCurrent;
		float dutyCycleNow;
		long rpm;
		float inpVoltage;
		float ampHours;
		float ampHoursCharged;
		long tachometer;
		long tachometerAbs;
	}dataPackage;

	/** Struct to hold the nunchuck values to send over UART */
		typedef struct  {
		int	valueX;
		int	valueY;
		bool upperButton; // valUpperButton
		bool lowerButton; // valLowerButton
	}nunchuckPackage;
	
int packSendPayload(uint8_t * payload, int lenPay);
bool processReadPacket(uint8_t * message,dataPackage * data );
bool getVescValues(void);
bool getVescValuesfwd(uint8_t canID );
void setNunchuckValues(void);
void unpackMessage(uint8_t * message,uint8_t lenMes,uint8_t * payload);
void VescUartSetRPM(int32_t rpm);
void VescUartSetRPMfwd(int32_t rpm,uint8_t canID);
#endif
