#include <stdio.h>

#include "data_decoding.h"

#ifndef DEBUG 
#define DEBUG 3
#endif

void init_decoding(){
	#if DEBUG
		printf("Entering init_decoding\n");
	#endif
	
	read_data = &ping;
	write_data = &pong;
}

void switch_read_write()
{
	#if DEBUG
		printf("Entering switch_read_write\n");
	#endif
	
	struct Data* temp = read_data;

	read_data = write_data;
	write_data = temp;
}

int data_update(uint8_t stream[])
{
	#if DEBUG
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
		return -1;
	}
	
	//check checksums
	for (i=1;i<length-2;i++)
	{
		checksum_1 += stream[i];
		checksum_2 += checksum_1;
	}
	
	//checksum1 is voorlaatste byte, checksu2 is last byte
	if(checksum_1 != stream[length-2] || checksum_2 != stream[length-1] || sender == 0)
	{	

		return -1;
	}
	
	
	return data_decode(0, sender, stream, length);
}

int data_decode(uint32_t pos, uint8_t sender,uint8_t stream[], int length) // start = 0
{
	#if DEBUG
		printf("Entering data_decode\n");
	#endif
	
	if(pos == 0)
	{
		pos = 3;
	} 
		
	switch(sender)
	{
		
		case 1: //sender_id of groundstation
			switch(stream[pos])
			{
				case 1: // servo commands - 28 bytes
					pos = data_write(stream, write_data->groundstation.commands.raw, 28, pos);
					write_data->groundstation.commands.message.new_data = 0;
					break;
				default: return -1; break;
			}
		break;
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
				default: return -1; break;
			}
		break;
		case 165: //sender_id of lisa
			switch(stream[pos]) // the message id of the folowing message
			{
				case 25: // Svinfo - NOG DOEN
					pos = data_write(stream, write_data->lisa_plane.svinfo.raw, 8, pos);
					write_data->lisa_plane.svinfo.message.new_data = 0;
					break;
				case 54: // airspeed - 48 bytes
					pos = data_write(stream, write_data->lisa_plane.airspeed.raw, 8, pos);
					write_data->lisa_plane.airspeed.message.new_data = 0;
					break;
				case 57: // airspeed_ets - 40 bytes
					pos = data_write(stream, write_data->lisa_plane.airspeed_ets.raw, 8, pos);
					write_data->lisa_plane.airspeed_ets.message.new_data = 0;
					break;
				case 155: // gps_int - 88 bytes
					pos = data_write(stream, write_data->lisa_plane.gps_int.raw, 8, pos);
					write_data->lisa_plane.gps_int.message.new_data = 0;
					break;
				case 203: // imu_gyro_raw - 48 bytes
					pos = data_write(stream, write_data->lisa_plane.imu_gyro_raw.raw, 12, pos);
					write_data->lisa_plane.imu_gyro_raw.message.new_data = 0;
					break;
				case 204: // imu_accel_raw - 48 bytes
					pos = data_write(stream, write_data->lisa_plane.imu_accel_raw.raw, 12, pos);
					write_data->lisa_plane.imu_accel_raw.message.new_data = 0;
					break;
				case 205: // imu_mag_raw - 48 bytes
					pos = data_write(stream, write_data->lisa_plane.imu_mag_raw.raw, 12, pos);
					write_data->lisa_plane.imu_mag_raw.message.new_data = 0;
					break;
				case 221: // baro_raw - 40 bytes
					pos = data_write(stream, write_data->lisa_plane.baro_raw.raw, 8, pos);
					write_data->lisa_plane.baro_raw.message.new_data = 0;
					break;
				default: return -1;break;
			}
		 break;
				default: return -1; break;
	}
	if (pos == length-2)
	{
		return 0; //data encoding succeeded
	} else {
		
		return data_decode(pos, sender, stream, length);
	}
}

int data_write(uint8_t stream[],uint8_t transfer[], int length, int pos)
{
	#if DEBUG
		printf("Entering data_write\n");
	#endif
	
	int i;
	for(i=0;i<length;i++)
	{
	    transfer[i] = stream[++pos]; //pos is first time message id so first ++ for first data byte
	}
	
	return pos + 1;
}

int data_encode(char buffer[])
{
	#if DEBUG
		printf("Entering data_encode\n");
	#endif
	
	int i = 0; 
	int checksum_1 = 0;
	int checksum_2 = 0;
	uint8_t length = 34; // with header and checksum
	uint8_t sender = 1;	
	
	buffer[0] = 0x99;
	buffer[1] = length;
	buffer[2] = 1; // sender id of server
	buffer[3] = 1; // message id

	for(i=0;i<28;i++)
	{
		buffer[4+i] = output.message[i];
	}

	for (i=1;i<length - 2;i++)
	{
		checksum_1 += buffer[i];
		checksum_2 += checksum_1;
	}
	
	buffer[32] = checksum_1;
	buffer[33] = checksum_2;
}

int data_encode_commands(int32_t commands[]){
	#if DEBUG
		printf("Entering data_encode_commands\n");
	#endif
	
	int i = 0; 
	int checksum_1 = 0;
	int checksum_2 = 0;
	
	for(i=0;i<7;i++)
	{
		write_data->commands_lisa_format.commands.message.servo_commands[i]=commands[i];
	}
	
	for (i=0;i<28;i++)
	{
		checksum_1 += write_data->commands_lisa_format.commands.raw[i];
		checksum_2 += checksum_1;
	}

	write_data->commands_lisa_format.commands.raw[28] = checksum_1;
	write_data->commands_lisa_format.commands.raw[29] = checksum_2;
	return 0;

}
