
//#define DEBUG

#include "vesc.h"
#include "../usart3.h"
#include "../usart1.h"
#include "libopencm3/cm3/cortex.h"

dataPackage data_vesc1;
dataPackage data_vesc2;
Custom_dataPackage Telemetry_data;
nunchuckPackage nunchuck; 
int RPM_VESC = 0;

extern SemaphoreHandle_t xSemaphore_usart3;

extern uint8_t payload_usart3[];
extern size_t lenPayload_usart3;

int packSendPayload(uint8_t * payload, int lenPay) {

	uint16_t crcPayload = crc16(payload, lenPay);
	int count = 0;
	uint8_t messageSend[256];

	if (lenPay <= 256)
	{
		messageSend[count++] = 2;
		messageSend[count++] = lenPay;
	}
	else
	{
		messageSend[count++] = 3;
		messageSend[count++] = (uint8_t)(lenPay >> 8);
		messageSend[count++] = (uint8_t)(lenPay & 0xFF);
	}

	memcpy(&messageSend[count], payload, lenPay);

	count += lenPay;
	messageSend[count++] = (uint8_t)(crcPayload >> 8);
	messageSend[count++] = (uint8_t)(crcPayload & 0xFF);
	messageSend[count++] = 3;
	messageSend[count] = '\0';

	

	// Sending package
	if( xSemaphoreTake( xSemaphore_usart3,( TickType_t ) 10 ) == pdTRUE ) // CHECK IF USART3 IS available
	{



		for(int i =0 ; i <= count; i++)
		{
			usart_send_blocking(USART3,messageSend[i]);
		}
	xSemaphoreGive( xSemaphore_usart3 );





	//usart1_printf("we give mutex usart3");
	}
	// Returns number of send bytes
	return count;
}


bool processReadPacket(uint8_t * message,dataPackage * data ) {

	COMM_PACKET_ID packetId;
	int32_t ind = 0;

	packetId = (COMM_PACKET_ID)message[0];
	message++; // Removes the packetId from the actual message (payload)

	switch (packetId){
		case COMM_GET_VALUES: // Structure defined here: https://github.com/vedderb/bldc/blob/43c3bbaf91f5052a35b75c2ff17b5fe99fad94d1/commands.c#L164
			ind = 4; // Skip the first 4 bytes 
			data->avgMotorCurrent 	= buffer_get_float32(message, 100.0, &ind);
			data->avgInputCurrent 	= buffer_get_float32(message, 100.0, &ind);
			ind += 8; // Skip the next 8 bytes
			data->dutyCycleNow 		= buffer_get_float16(message, 1000.0, &ind);
			data->rpm 				= buffer_get_int32(message, &ind);
			data->inpVoltage 		= buffer_get_float16(message, 10.0, &ind);
			data->ampHours 			= buffer_get_float32(message, 10000.0, &ind);
			data->ampHoursCharged 	= buffer_get_float32(message, 10000.0, &ind);
			ind += 8; // Skip the next 8 bytes 
			data->tachometer 		= buffer_get_int32(message, &ind);
			data->tachometerAbs 		= buffer_get_int32(message, &ind);
			return true;

		break;

		default:
			return false;
		break;
	}
}

bool getVescValues(void) {
	uint8_t command[1] = { COMM_GET_VALUES };
	uint8_t message[100];
	uint8_t payload[100];
	lenPayload_usart3 =  0;

	packSendPayload(command, 1);

	//delay(1); //needed, otherwise data is not read
	for (int i = 0; i < 3000000; i++)
				__asm__("nop");




	if (lenPayload_usart3 > 55) { //if more than 55 bytes
		#ifdef DEBUG
		usart1_printf("getVescValues : evaluate buffer \n");
		#endif
			memcpy(message,payload_usart3,lenPayload_usart3);
			uint8_t length = lenPayload_usart3;
			lenPayload_usart3 = 0;
			unpackMessage(message,length,payload);



			bool read = processReadPacket(payload,&data_vesc1); //returns true if sucessfull

			return read;
		}
	else
	{
		return false;
	}

}

bool getVescValuesfwd(uint8_t canID ) {
	uint8_t index = 0;
	uint8_t command[3];
	uint8_t message[100];
	uint8_t payload[100];
	 command[index++] =  COMM_FORWARD_CAN ; //Forwarding CAN 
	 command[index++] = canID;                //Sending CAN id
	 command[index++] =  COMM_GET_VALUES ;  //Requesting Values
	lenPayload_usart3 = 0;
	packSendPayload(command,3);
	// delay(1); //needed, otherwise data is not read
	for (int i = 0; i < 3000000; i++)
					__asm__("nop");




		if (lenPayload_usart3 > 55) { //if more than 55 bytes
			#ifdef DEBUG
			usart1_printf("getVescValues : evaluate buffer \n");
			#endif
				memcpy(message,payload_usart3,lenPayload_usart3);
				uint8_t length = lenPayload_usart3;
				lenPayload_usart3 = 0;
				unpackMessage(message,length,payload);

		bool read = processReadPacket(payload,&data_vesc2); //returns true if sucessfull

		return read;
	}
	else
	{
		return false;
	}
}

void setNunchuckValues(void) {
	int32_t ind = 0;
	uint8_t payload[11];

	
	payload[ind++] = COMM_SET_CHUCK_DATA;
	payload[ind++] = nunchuck.valueX;
	payload[ind++] = nunchuck.valueY;
	buffer_append_bool(payload, nunchuck.lowerButton, &ind);
	buffer_append_bool(payload, nunchuck.upperButton, &ind);
	
	// Acceleration Data. Not used, Int16 (2 byte)
	payload[ind++] = 0;
	payload[ind++] = 0;
	payload[ind++] = 0;
	payload[ind++] = 0;
	payload[ind++] = 0;
	payload[ind++] = 0;

	

	packSendPayload(payload, 11);
}

void VescUartSetRPM(int32_t rpm) {
	int32_t index = 0;
	uint8_t payload[5];

	payload[index++] = COMM_SET_RPM ;
	buffer_append_int32(payload, rpm , &index);
	packSendPayload(payload, 5);
}

void VescUartSetRPMfwd(int32_t rpm,uint8_t canID) {
	int32_t index = 0;
	uint8_t payload[7];
	payload[index++] =  COMM_FORWARD_CAN ; //Forwarding CAN
	payload[index++] = canID;
	payload[index++] = COMM_SET_RPM ;
	buffer_append_int32(payload, rpm , &index);
	packSendPayload(payload, 7);
}

void unpackMessage(uint8_t * message,uint8_t lenMes,uint8_t * payload)
{

	uint8_t endMessage = 0 ;
	uint8_t lenPayload = 0 ;

	uint16_t crcMessage = 0;
	uint16_t crcPayload = 0;

	switch (message[0])
	{
		case 2: //short frame <255
			endMessage = message[1] + 5; //Payload size + 2 for sice + 3 for CRC and End.
			lenPayload = message[1];
			#ifdef DEBUG
			usart1_printf("frame type : short frame \n");
			#endif
			if(message[endMessage - 1] == 3)
			{
				#ifdef DEBUG
				usart1_printf("Message seems to be complete (ending by 3) \n");
				#endif
			}
			else
			{
				#ifdef DEBUG
				usart1_printf("Message is broken...(not ending by 3)\n");
				#endif
			}
		break;

		case 3: // long frame >255
			#ifdef DEBUG
			lenPayload = 0; // To remove warning unused
			usart1_printf("long frame are not yet supported \n");
			#endif

		break;

		default: // invalid message
			#ifdef DEBUG
			usart1_printf("Unvalid start bit \n");
			#endif

		break;
	}

	// now we will extract the payload then check CRC

	// Rebuild crc:
	crcMessage = message[(lenMes) - 3] << 8;
	crcMessage &= 0xFF00;
	crcMessage += message[(lenMes) - 2];


	// Extract payload:
	memcpy(payload, &message[2], message[1]);

	crcPayload = crc16(payload, message[1]);

	if(crcPayload == crcMessage)
	{
		#ifdef DEBUG
		usart1_printf("Message CRC and Payload CRC match ! \n");
		#endif
	}
	else
	{
		#ifdef DEBUG
		usart1_printf("CRC fucked up ! \n");
		#endif
	}
	#ifdef DEBUG
	usart1_printf("message length : %d \n",lenMes);
	usart1_printf("payload length : %d \n",lenPayload);
	usart1_printf("endMessage : %d \n",endMessage);
	#endif



}






