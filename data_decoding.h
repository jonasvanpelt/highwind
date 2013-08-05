#ifndef DATA_DECODING_H_ 
#define DATA_DECODING_H_

#include <stdint.h>
#include <time.h>
#include <sys/time.h>

/********************************
 * GLOBALS
 * ******************************/

//OUTPUT BUFFER
union Output{
	int32_t raw[7];
	uint8_t message[28]; // number of servos times 4
} output;

//INPUT
struct Data {
	//only for debugging, to send output commands to myself to test 
	struct Groundstation { // sender id = 1
		union Commands{ // id = 1
			uint8_t raw[64];// always use sizeof
			struct Commands_message{
				int32_t servo_commands[7];
				time_t time; // size 8 bytes
				struct timeval tv; // size 16 bytes
				int8_t new_data;
			} message;
		} commands;
	} groundstation;
	struct Commands_lisa_format{
		union Commands_send{
			uint8_t raw[30];
			struct{
				int32_t servo_commands[7];
				uint8_t checksum1;
				uint8_t checksum2;
			}message;
		}commands;	
	}commands_lisa_format;
	//for status and error messages coming from beaglebone on plane
	struct Bone_plane { // sender id = 2
		union Status { // id = 1
			uint8_t raw[155];
			struct Status_message {
				uint8_t error;
				time_t time;
				struct timeval tv;
				int8_t new_data;
			} message;
		} status;
		union Error { // id = 2
			uint8_t raw[1];
			struct Error_message {
				uint8_t error;
				time_t time;
				struct timeval tv;
				int8_t new_data;
			} message;
		} error;
	} bone_plane;
	//to decode error messages coming from lisa
	struct Lisa_plane { // sender id = 165
		union Airspeed { // id = 54
			uint8_t raw[9];
			struct Airspeed_message {
				float airspeed;
				float airspeed_sp;
				float airspeed_cnt;
				float groundspeed_sp;
				time_t time; 
				struct timeval tv; 
				int8_t new_data;
			} message;
		} airspeed;
		union Svinfo { // id = 25
			uint8_t raw[33];
			struct Svinfo_message {
				uint8_t chn;
				uint8_t svid;
				uint8_t flags;
				uint8_t qi;
				uint8_t cno;
				int8_t elev;
				uint16_t azim;
				time_t time; 
				struct timeval tv; 
				int8_t new_data;
			} message;
		} svinfo;
		union Airspeed_ets { // id = 57
			uint8_t raw[9];
			struct Airspeed_ets_message {
				uint16_t adc;
				uint16_t offset;
				float scaled;
				time_t time;
				struct timeval tv;
				int8_t new_data;
			} message;
		} airspeed_ets;
		union Gps_int { // id = 155
			uint8_t raw[9];
			struct Gps_int_message {
				int32_t ecef_x;
				int32_t ecef_y;
				int32_t ecef_z;
				int32_t lat;
				int32_t lon;
				int32_t alt;
				int32_t hmsl;
				int32_t ecef_xd;
				int32_t ecef_yd;
				int32_t ecef_zd;
				int32_t pacc;
				int32_t sacc;
				uint32_t tow;
				uint16_t pdop;
				uint8_t numsv;
				uint8_t fix;
				time_t time;
				struct timeval tv;
				int8_t new_data;
			} message;
		} gps_int;
		union Baro_raw { // id = 221
			uint8_t raw[9];
			struct Baro_raw_message {
				int32_t abs;
				int32_t diff;
				time_t time;
				struct timeval tv;
				int8_t new_data;
			} message;
		} baro_raw;
		union Imu_gyro_raw { // id = 203
			uint8_t raw[9];
			struct Imu_gyro_message {
				int32_t gp;
				int32_t gq;
				int32_t gr;
				time_t time;
				struct timeval tv;
				int8_t new_data;
			} message;
		} imu_gyro_raw;
		union Imu_accel_raw { // id = 204
			uint8_t raw[9];
			struct Imu_accel_message {
				int32_t ax;
				int32_t ay;
				int32_t az;
				time_t time;
				struct timeval tv;
				int8_t new_data;
			} message;
		} imu_accel_raw;
		union Imu_mag_raw { // id = 205
			uint8_t raw[9];
			struct Imu_mag_message {
				int32_t mx;
				int32_t my;
				int32_t mz;
				time_t time;
				struct timeval tv;
				int8_t new_data;
			} message;
		} imu_mag_raw;
	} lisa_plane;
};

//one writes to ping and another can read data from pong and upside down
struct Data ping;
struct Data pong;

//pointers to ping and pong
struct Data* read_data;
struct Data* write_data;

/********************************
 * PROTOTYPES
 * ******************************/
void init_decoding(void);
int data_update(uint8_t stream[]);
int data_decode(uint32_t pos, uint8_t sender,uint8_t stream[], int length);
int32_t data_write(uint8_t stream[], uint8_t data[], int length, int pos);
void switch_read_write(void);
int data_encode_commands(int32_t buffer[]);

#endif /*DATA_DECODING_H_*/
