/*
 * AUTHOR:Jonas Van Pelt and Maarten Arits
 */
 
#include <stdlib.h>
#include "header_files/data_decoding.h"
#include <string.h>

#ifndef DEBUG 
#define DEBUG 0
#endif

#if DEBUG
#include <stdio.h>
#endif

/********************************
 * PROTOTYPES PRIVATE
 * ******************************/
static DEC_errCode data_to_struct(uint8_t sender,uint8_t stream[], int length); 
void data_write(uint8_t stream[],void *destination, int length);

/********************************
 * GLOBALS
 * ******************************/
 
//one writes to ping and another can read data from pong and upside down
static Data ping;
static Data pong;

//pointers to ping and pong
static Data* read_data;
static Data* write_data;


/********************************
 * FUNCTIONS
 * ******************************/

void init_decoding(){
	#if DEBUG > 1
		printf("Entering init_decoding\n");
	#endif
	
	read_data = &ping;
	write_data = &pong;
}

void switch_read_write()
{
	#if DEBUG  > 1
		printf("Entering switch_read_write\n");
	#endif
	
	Data* temp = read_data;

	read_data = write_data;
	write_data = temp;
}

DEC_errCode data_decode(uint8_t stream[])
{
	#if DEBUG  > 1
		printf("Entering data_decode\n");
	#endif
	
	int i = 0; 
	uint8_t checksum_1 = 0;
	uint8_t checksum_2 = 0;
	uint8_t length = stream[LENGTH_INDEX];
	uint8_t sender = stream[SENDER_ID_INDEX];	

	//check first bit is 0x99
	if(stream[STARTBYTE_INDEX] != 0x99)
	{
		//unknown package !!!
		return DEC_ERR_START_BYTE;
	}
	
	calculate_checksum(stream,&checksum_1,&checksum_2);
	
	//checksum1 is voorlaatste byte, checksum2 is last byte
	if(checksum_1 != stream[length-2] || checksum_2 != stream[length-1])
	{	
		return DEC_ERR_CHECKSUM;
	}
	
	return data_to_struct(sender, stream, length);
}

static DEC_errCode data_to_struct(uint8_t sender,uint8_t stream[], int length) // start = 0
{
	#if DEBUG  > 1
		printf("Entering data_to_struct\n");
	#endif
	
	switch(sender)
	{
		case BEAGLEBONE: //sender_id of beaglebone
			switch(stream[MESSAGE_ID_INDEX])
			{
				case BEAGLE_ERROR: 
					data_write(stream, (void *)&write_data->bone_plane.error, sizeof(Beagle_error)-1);
					write_data->bone_plane.error.new_data = 0;
					break;
				default: return DEC_ERR_UNKNOWN_BONE_PACKAGE; break;
			}
		break;
		case LISA: //sender_id of lisa
			switch(stream[MESSAGE_ID_INDEX]) // the message id of the folowing message
			{
				case SVINFO:
					data_write(stream, (void *)&write_data->lisa_plane.svinfo, sizeof(Svinfo)-1);
					break;
				case SYSMON: 
					data_write(stream, (void *)&write_data->lisa_plane.sys_mon, sizeof(Sys_mon)-1);
					break;
				case AIRSPEED_ETS: 
					data_write(stream, (void *)&write_data->lisa_plane.airspeed_ets, sizeof(Airspeed_ets)-1);
					break;
				case ACTUATORS:
					data_write(stream, (void *)&write_data->lisa_plane.actuators, sizeof(Actuators)-1);
					break;
				case GPS_INT: 
					data_write(stream, (void *)&write_data->lisa_plane.gps_int, sizeof(Gps_int)-1);
					break;
				case IMU_GYRO_RAW:
					data_write(stream, (void *)&write_data->lisa_plane.imu_gyro_raw, sizeof(Imu_gyro_raw)-1);
					break;
				case IMU_ACCEL_RAW: 
					data_write(stream, (void *)&write_data->lisa_plane.imu_accel_raw, sizeof(Imu_accel_raw)-1);
					break;
				case IMU_MAG_RAW: 
					data_write(stream, (void *)&write_data->lisa_plane.imu_mag_raw, sizeof(Imu_mag_raw)-1);
					break;
				case UART_ERRORS:
					data_write(stream, (void *)&write_data->lisa_plane.uart_errors, sizeof(UART_errors)-1);
					break;
				case BARO_RAW:
					data_write(stream, (void *)&write_data->lisa_plane.baro_raw, sizeof(Baro_raw)-1);
					break;
				default: return DEC_ERR_UNKNOWN_LISA_PACKAGE;break;
			}
		 break;
				default: return DEC_ERR_UNKNOWN_SENDER; break;
	}	
	return DEC_ERR_NONE;	
}

void data_write(uint8_t stream[],void *destination, int length)
{
	#if DEBUG  > 1
		printf("Entering data_write\n");
	#endif

	memcpy(destination,&(stream[MESSAGE_START_INDEX]),length);	
}

DEC_errCode data_encode(uint8_t message[],long unsigned int message_length,uint8_t encoded_data[],int sender_id,int message_id)
{
	#if DEBUG  > 1
		printf("Entering data_encode\n");
	#endif
	
	int i = 0,j; 
	uint8_t checksum_1 = 0;
	uint8_t checksum_2 = 0;
	uint8_t length = message_length+6+16; //message length + 6 info bytes + 16 timestamp bytes
	timeval timestamp; 
	 
	encoded_data[STARTBYTE_INDEX] = 0x99;
	encoded_data[LENGTH_INDEX] = length;
	encoded_data[SENDER_ID_INDEX] = sender_id; // sender id of server
	encoded_data[MESSAGE_ID_INDEX] = message_id; // message id

	//add message
	memcpy(&(encoded_data[MESSAGE_START_INDEX]),(void *)message,message_length);

	//get localtime 
	gettimeofday(&timestamp, NULL);
	
	//add timestamp to buffer
	memcpy(&(encoded_data[message_length+4]),(void *)&timestamp,sizeof(timestamp));
	
	calculate_checksum(encoded_data,&checksum_1,&checksum_2);
	
	encoded_data[length-2] = checksum_1;
	encoded_data[length-1] = checksum_2;
	
	return DEC_ERR_NONE;
}


Data* get_read_pointer()
{
	#if DEBUG  > 1
		printf("Entering get_read_pointer\n");
	#endif
		return read_data;
}

void calculate_checksum(uint8_t buffer[],uint8_t *checksum_1,uint8_t *checksum_2){
	#if DEBUG  > 1
		printf("Entering calculate_checksum\n");
	#endif
	int i;
	int length = buffer[1];
	*checksum_1=0;
	*checksum_2=0;
	
	//start byte '0x99' is not in checksum
	for (i=1;i<length-2;i++)
	{
		*checksum_1 += buffer[i];
		*checksum_2 += *checksum_1;
	}
}

int add_timestamp(uint8_t buffer[]){
	#if DEBUG  > 1
		printf("Entering add_timestamp\n");
	#endif
	
	int length_original=buffer[1],i,j;
	uint8_t checksum_1,checksum_2;
	int new_length=length_original+16; //timeval is 16 bytes
	struct timeval tv_8;
	TimestampBeagle timestampBeagle;

	//get localtime 
	gettimeofday(&tv_8, NULL);

	//convert beaglebone 8 byte timeval to 16 byte timeval for server
	timestampBeagle.tv.tv_sec=(uint64_t)tv_8.tv_sec;
	timestampBeagle.tv.tv_usec=(uint64_t)tv_8.tv_usec;

	//update message length
	buffer[1]=new_length; 
	
	//add timestamp to buffer
	memcpy(&(buffer[length_original-2]),(void *)&timestampBeagle.tv,sizeof(timestampBeagle.tv)); //overwrite previous checksums (-2)
	
	//recalculate checksum
	calculate_checksum(buffer,&checksum_1,&checksum_2);
	buffer[new_length-2]=checksum_1;
	buffer[new_length-1]=checksum_2;
	
	return new_length;
}

int strip_timestamp(uint8_t buffer[]){
	#if DEBUG  > 1
		printf("Entering strip_timestamp\n");
	#endif
	
	int length=buffer[1],i,j;
	uint8_t checksum_1,checksum_2;
	int new_length=length-16; //timeval is 16 bytes

	//update message length
	buffer[1]=new_length; 
	
	//recalculate checksum
	calculate_checksum(buffer,&checksum_1,&checksum_2);
	buffer[new_length-2]=checksum_1;
	buffer[new_length-1]=checksum_2;
	
	return new_length;	
}







