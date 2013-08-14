#include <stdlib.h>
#include "data_decoding.h"


#ifndef DEBUG 
#define DEBUG 0
#endif

//#if DEBUG
#include <stdio.h>
//#endif

/********************************
 * PROTOTYPES PRIVATE
 * ******************************/
DEC_errCode data_decode(uint32_t pos, uint8_t sender,uint8_t stream[], int length);


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

DEC_errCode data_update(uint8_t stream[])
{
	#if DEBUG  > 1
		printf("Entering data_update\n");
	#endif
	
	int i = 0; 
	uint8_t checksum_1 = 0;
	uint8_t checksum_2 = 0;
	uint8_t length = stream[1];
	uint8_t sender = stream[2];	

	//check first bit is 0x99
	if(stream[0] != 0x99)
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
	
	
	return data_decode(0, sender, stream, length);
}

DEC_errCode data_decode(uint32_t pos, uint8_t sender,uint8_t stream[], int length) // start = 0
{
	#if DEBUG  > 1
		printf("Entering data_decode\n");
	#endif
	
	if(pos == 0)
	{
		pos = 3;
	} 
		
	switch(sender)
	{
		case 2: //sender_id of beaglebone
			switch(stream[pos])
			{
				case 1: // status - x bytes
					pos = data_write(stream, write_data->bone_plane.status.raw, 8, pos);
					write_data->bone_plane.status.message.new_data = 0;
					break;
				case 2: // error - 1 byte
					pos = data_write(stream, write_data->bone_plane.error.raw, 1, pos);
					write_data->bone_plane.error.message.new_data = 0;
					break;
				default: return DEC_ERR_UNKNOWN_BONE_PACKAGE; break;
			}
		break;
		case 165: //sender_id of lisa
			switch(stream[pos]) // the message id of the folowing message
			{
				case 25: // Svinfo 
					pos = data_write(stream, write_data->lisa_plane.svinfo.raw, 24, pos);
					write_data->lisa_plane.svinfo.message.new_data = 0;
					break;
				/*case 54: // airspeed - 48 bytes
					pos = data_write(stream, write_data->lisa_plane.airspeed.raw, 24, pos);
					write_data->lisa_plane.airspeed.message.new_data = 0;
					break;*/
				case 57: // airspeed_ets - 40 bytes
					pos = data_write(stream, write_data->lisa_plane.airspeed_ets.raw, 24, pos);
					write_data->lisa_plane.airspeed_ets.message.new_data = 0;
					break;
				case 155: // gps_int - 81 bytes
					pos = data_write(stream, write_data->lisa_plane.gps_int.raw, 72, pos);
					write_data->lisa_plane.gps_int.message.new_data = 0;
					break;
				case 203: // imu_gyro_raw - 48 bytes
					pos = data_write(stream, write_data->lisa_plane.imu_gyro_raw.raw, 28, pos);
					write_data->lisa_plane.imu_gyro_raw.message.new_data = 0;
					break;
				case 204: // imu_accel_raw - 48 bytes
					pos = data_write(stream, write_data->lisa_plane.imu_accel_raw.raw, 28, pos);
					write_data->lisa_plane.imu_accel_raw.message.new_data = 0;
					break;
				case 205: // imu_mag_raw - 48 bytes
					pos = data_write(stream, write_data->lisa_plane.imu_mag_raw.raw, 28, pos);
					write_data->lisa_plane.imu_mag_raw.message.new_data = 0;
					break;
				case 221: // baro_raw - 40 bytes
					pos = data_write(stream, write_data->lisa_plane.baro_raw.raw, 24, pos);
					write_data->lisa_plane.baro_raw.message.new_data = 0;
					break;
				default: return DEC_ERR_UNKNOWN_LISA_PACKAGE;break;
			}
		 break;
				default: return DEC_ERR_UNKNOWN_SENDER; break;
	}
	if (pos == length-2)
	{
		return DEC_ERR_NONE; //data encoding succeeded
	} else {
		printf("here\n");
		return data_decode(pos, sender, stream, length);
	}
}

int data_write(uint8_t stream[],uint8_t transfer[], int length, int pos)
{
	#if DEBUG  > 1
		printf("Entering data_write\n");
	#endif
	
	int i;
	for(i=0;i<length;i++)
	{
	    transfer[i] = stream[++pos]; //pos is first time message id so first ++ for first data byte
	}
	
	return pos + 1;
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
	Timestamp timestamp;
		
	encoded_data[0] = 0x99;
	encoded_data[1] = length;
	encoded_data[2] = sender_id; // sender id of server
	encoded_data[3] = message_id; // message id

	/*printf("message:");
	for(i=0;i<message_length;i++){
		printf("%d ",message[i]);
	}
	printf("\n");*/

	//add message
	for(i=0;i<message_length;i++)
	{
		encoded_data[4+i] = message[i];
	}
	
	//add timestamp
	//add timestamp to buffer
	j=0;
	for(i=message_length+4;i<length-2;i++){ //overwrite previous checksums
		encoded_data[i]=timestamp.raw[j];j++;	
	}
	
	calculate_checksum(encoded_data,&checksum_1,&checksum_2);
	
	encoded_data[length-2] = checksum_1;
	encoded_data[length-1] = checksum_2;
	
	/*printf("raw:");
	for(i=0;i<length;i++){
		printf("%d ",encoded_data[i]);
	}
	printf("\n");*/
	
	return DEC_ERR_NONE;
}


Data* get_read_pointer()
{
		return read_data;
}

void calculate_checksum(uint8_t buffer[],uint8_t *checksum_1,uint8_t *checksum_2){
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
	j=0;
	for(i=length_original-2;i<new_length-2;i++){ //overwrite previous checksums
		buffer[i]=timestampBeagle.raw[j];j++;	
	}
	
	//recalculate checksum
	calculate_checksum(buffer,&checksum_1,&checksum_2);
	buffer[new_length-2]=checksum_1;
	buffer[new_length-1]=checksum_2;
	
	return new_length;
}

int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
    long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;
    return (diff<0);	//Return 1 if the difference is negative, otherwise 0. 
}

void timestamp_to_timeString(struct timeval tv,char time_string[]){	
	time_t nowtime;
	struct tm *nowtm;
	char tmbuf[64];
	nowtime = tv.tv_sec;
	nowtm = localtime(&nowtime);
	strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);
	snprintf(time_string, 64, "%s.%06d", tmbuf, (int)tv.tv_usec);
}

