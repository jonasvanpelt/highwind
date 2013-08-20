#ifndef DATA_DECODING_H_ 
#define DATA_DECODING_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <sys/time.h>
#include <time.h>

/********************************
 * GLOBALS
 * ******************************/
 
// Decoding error codes 
enum dec_errCode { DEC_ERR_NONE = 0,DEC_ERR_START_BYTE,DEC_ERR_CHECKSUM,DEC_ERR_UNKNOWN_BONE_PACKAGE,DEC_ERR_UNKNOWN_LISA_PACKAGE,DEC_ERR_UNKNOWN_SENDER,DEC_ERR_UNDEFINED};
typedef enum dec_errCode DEC_errCode;

 
enum Library {UDP_L,UART_L,DECODE_L,LOG_L,CIRCULAR_BUFFER_L};
typedef enum Library library; 

typedef struct timeval timeval;

typedef union{
		uint8_t raw[16];
		timeval tv;
} Timestamp;

typedef struct {
		uint64_t tv_sec;
		uint64_t tv_usec;
} Timeval16; //redefine a new 16byte timeval for beaglebone because beaglebone has 8 byte timeval

typedef union{
		uint8_t raw[16];
		Timeval16 tv;
} TimestampBeagle;

//OUTPUT 
typedef union{ //message id 72
	uint8_t raw[14]; 
	struct Output_message {
			int16_t servo_1;
			int16_t servo_2;
			int16_t servo_3;
			int16_t servo_4;
			int16_t servo_5;
			int16_t servo_6;
			int16_t servo_7;
		} message;
} Output;

	
typedef union { // id = 1
		uint8_t raw[18];
		struct Status_message {
			uint8_t error;
			timeval tv;
			int8_t new_data;
		} message;
	} Status;
	
typedef union { // id = 2
		uint8_t raw[19];
		struct Error_message {
			uint8_t library;
			uint8_t error;
			timeval tv; //16
			int8_t new_data;
		} message;
	} Beagle_error;
	
typedef struct { // sender id = 2
		Status status;
		Beagle_error error;
	} Bone_plane;	
	
//these structure are translated from messages.xml in the paparazzi code
	
typedef union { // id = 54
		uint8_t raw[33];
		struct Airspeed_message {
			float airspeed;
			float airspeed_sp;
			float airspeed_cnt;
			float groundspeed_sp;
			timeval tv; 
			int8_t new_data;
		} message;
	} Airspeed;
	
typedef union { // id = 33
		uint8_t raw[28];
		struct Sys_mon_message {
			uint16_t periodic_time;
			uint16_t periodic_cycle;
			uint16_t periodic_cycle_min;
			uint16_t periodic_cycle_max;
			uint16_t event_number;
			uint8_t cpu_load; //in %
			timeval tv; 
			int8_t new_data;
		} message;
	} Sys_mon;
	
typedef union { // id = 208
		uint8_t raw[24];
		struct UART_errors_message {
			uint16_t overrun_cnt;
			uint16_t noise_err_cnt;
			uint16_t framing_err_cnt;
			uint8_t bus_number; 
			timeval tv; 
			int8_t new_data;
		} message;
	} UART_errors;
	
typedef union { // id = 105
		uint8_t raw[32];
		struct Actuators_message {
			uint8_t arr_length;
			int16_t values[7]; //the ACTUATORS message contains the final commands to the servos (or any actuator) regardless of which mode you are in (e.g. if it's comming from RC or NAV)
			timeval tv; 
			int8_t new_data;
		} message;
	} Actuators;
	
typedef union { // id = 25
		uint8_t raw[25];
		struct Svinfo_message {
			uint8_t chn;
			uint8_t svid;
			uint8_t flags;
			uint8_t qi;
			uint8_t cno;
			int8_t elev;
			uint16_t azim;
			timeval tv; 
			int8_t new_data;
		} message;
	} Svinfo;
	
typedef union Airspeed_ets { // id = 57
		uint8_t raw[25];
		struct Airspeed_ets_message {
			uint16_t adc;
			uint16_t offset;
			float scaled; //4
			timeval tv; //16
			int8_t new_data;
		} message;
	} Airspeed_ets;
	
typedef union { // id = 155
		uint8_t raw[73];
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
			timeval tv; //16
			int8_t new_data;
		} message;
	} Gps_int;
	
typedef union { // id = 221
		uint8_t raw[25];
		struct Baro_raw_message {
			int32_t abs;
			int32_t diff;
			timeval tv;
			int8_t new_data;
		} message;
	} Baro_raw;
	
typedef union { // id = 203
		uint8_t raw[29];
		struct Imu_gyro_message {
			int32_t gp;
			int32_t gq;
			int32_t gr;
			timeval tv;
			int8_t new_data;
		} message;
	} Imu_gyro_raw;
	
typedef union { // id = 204
		uint8_t raw[29];
		struct Imu_accel_message  {
			int32_t ax;
			int32_t ay;
			int32_t az;
			Timeval16 tv;
			int8_t new_data;
		} message;
	} Imu_accel_raw;
	
typedef union { // id = 205
		uint8_t raw[29];
		struct Imu_mag_message {
			int32_t mx;
			int32_t my;
			int32_t mz;
			timeval tv;
			int8_t new_data;
		} message;
	} Imu_mag_raw;
	
typedef struct { // sender id = 165
		Airspeed airspeed;
		Svinfo svinfo;
		Airspeed_ets airspeed_ets;
		Gps_int gps_int;
		Baro_raw baro_raw;
		Imu_gyro_raw imu_gyro_raw;
		Imu_accel_raw imu_accel_raw;
		Imu_mag_raw imu_mag_raw;
		Sys_mon sys_mon;
		UART_errors uart_errors;
		Actuators actuators;
	} Lisa_plane;	
	
//INPUT
typedef struct
{
	Bone_plane bone_plane;
	Lisa_plane lisa_plane;
} Data;

/********************************
 * PROTOTYPES PUBLIC
 * ******************************/
extern void init_decoding(void);
extern DEC_errCode data_update(uint8_t stream[]);
extern int32_t data_write(uint8_t stream[], uint8_t data[], int length, int pos);
extern void switch_read_write(void);
extern DEC_errCode data_encode(uint8_t message[],long unsigned int message_length,uint8_t encoded_data[],int sender_id,int message_id);
extern Data* get_read_pointer();
extern void calculate_checksum(uint8_t buffer[],uint8_t *checksum_1,uint8_t *checksum2);
extern int add_timestamp(uint8_t buffer[]);
extern int strip_timestamp(uint8_t buffer[]);
extern void timestamp_to_timeString(Timeval16 tv,char time_string[]);
extern int timeval_subtract(Timeval16 *result,Timeval16 *t2, Timeval16 *t1);

#ifdef __cplusplus
}
#endif

#endif /*DATA_DECODING_H_*/
