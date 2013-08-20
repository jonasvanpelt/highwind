#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <endian.h>

#include "udp_communication.h"
#include "data_decoding.h"
#include "log.h"


#define MAX_INPUT_STREAM_SIZE 255
#define MAX_OUTPUT_STREAM_SIZE 36
#define UDP_SOCKET_TIMEOUT 1000000000


#ifndef DEBUG 
#define DEBUG 0
#endif

		
/************************************
 * PROTOTYPES
 * **********************************/

void *server_to_planebone(void *connection);
static void DEC_err_handler(DEC_errCode err ); 
static void UDP_err_handler( UDP_errCode err ); 
static void err_receiver();
void print_mem(void const *vp, int n);
void print_latency(timeval tvSend);


 /***********************************
  * GLOBALS
  * *********************************/
  
static char FILENAME[] = "main_full_server.c";

    
typedef struct{
		int port_number_server_to_planebone;
		int port_number_planebone_to_server;
		char *planebone_ip;
} Connection;


 /***********************************
  * MAIN
  * *********************************/
int main(int argc, char *argv[]){
	Connection connection;

	//parse arguments	
	if(argc == 4){
		//first argument is always name of program or empty string
		connection.planebone_ip=argv[1];
		connection.port_number_server_to_planebone=atoi(argv[2]);	
		connection.port_number_planebone_to_server=atoi(argv[3]);
	}else{
			printf("wrong parameters: planebone ip - send port number - receive port number\n");
			exit(EXIT_FAILURE);		
	}
		
	pthread_t thread_server_to_planebone;

	//create a second thread which executes server_to_planebone
	if(pthread_create(&thread_server_to_planebone, NULL, server_to_planebone,&connection)) {
		error_write(FILENAME,"error creating lisa thread");
		exit(EXIT_FAILURE);
	}	
	
	/*-------------------------START OF FIRST THREAD: PLANEBONE TO SERVER------------------------*/
	
	static UDP udp_server;
	uint8_t input_stream[MAX_INPUT_STREAM_SIZE];

	UDP_err_handler(openUDPServerSocket(&udp_server,connection.port_number_planebone_to_server,UDP_SOCKET_TIMEOUT));
	
	//init the data decode pointers
	init_decoding();
	
	/*
	 * WHAT WE EXPECT:
	 * IMU_ACCEL_RAW 204
	 * IMU_GYRO_RAW 203
	 * IMU_MAG_RAW 205
	 * BARO_RAW 221
	 * GPS_INT 155
	 * AIRSPEED_ETS 57
	 * SYSMON 33
	 * UART_ERROR 208
	 * ACTUATORS_received 105
	 * */
	 
	int IMU_ACCEL_RAW_received=0;
	int IMU_GYRO_RAW_received=0;	
	int IMU_MAG_RAW_received=0;
	int BARO_RAW_received=0;
	int GPS_INT_received=0;
	int AIRSPEED_received=0;
	int SVINFO_received=0;
	int SYSMON_received=0;
	int UART_ERROR_received=0;
	int ACTUATORS_received=0;
	int err;
	

	while(1){
		//1. retreive UDP data form planebone from ethernet port.
		err = receiveUDPServerData(&udp_server,(void *)&input_stream,sizeof(input_stream)); //blocking !!!
		UDP_err_handler(err); 
	
		if(err == UDP_ERR_NONE){
			#if DEBUG > 0
			
			printf("message raw: ");
			int i;
			for(i=0;i<input_stream[1];i++){
					printf("%d ",input_stream[i]);
			}
			printf("\n");
			
			printf("start hex: %x\n", input_stream[0]);
			printf("length: %d\n", input_stream[1]);
			printf("send id: %d\n", input_stream[2]);
			printf("message id: %d\n", input_stream[3]);
			printf("checksum1: %d\n", input_stream[input_stream[1]-2]);
			printf("checksum2: %d\n", input_stream[input_stream[1]-1]);
			printf("\n");
			
			#endif
			
			//2. decode data 
			
			int err  = data_update(input_stream);
			DEC_err_handler(err);

			
			if(err==DEC_ERR_NONE){ 
			
				switch_read_write(); //only switch read write if data decoding was succesfull
				Data* data = get_read_pointer();

				if(input_stream[3]==203){
					IMU_GYRO_RAW_received=1;
				}
				else if(input_stream[3]==204){
					IMU_ACCEL_RAW_received=1;
				}
				else if(input_stream[3]==205){
					IMU_MAG_RAW_received=1;
				}
				else if(input_stream[3]==221){
					BARO_RAW_received=1;
				}
				else if(input_stream[3]==155){
					GPS_INT_received=1;
				}
				else if(input_stream[3]==57){
					AIRSPEED_received=1;
				}else if(input_stream[3]==25){
					SVINFO_received=1;
				}else if(input_stream[3]==33){
					SYSMON_received=1;
				}else if(input_stream[3]==208){
					UART_ERROR_received=1;
				}else if(input_stream[3]==105){
					ACTUATORS_received=1;
				}else{
						printf("UNKNOWN DATA with id %d\n",input_stream[3]);
						exit(1);
				}
				
				/*printf("IMU_GYRO_RAW_received %d\n",IMU_GYRO_RAW_received);
				printf("IMU_ACCEL_RAW_received %d\n",IMU_ACCEL_RAW_received);
				printf("IMU_MAG_RAW_received %d\n",IMU_MAG_RAW_received);
				printf("BARO_RAW_received %d\n",BARO_RAW_received);
				printf("GPS_INT_received %d\n",GPS_INT_received);			
				printf("AIRSPEED_received %d\n",AIRSPEED_received);			
				printf("SVINFO_received %d\n",SVINFO_received);	
				printf("SYSMON_received %d\n",SYSMON_received);	
				printf("UART_ERROR_received %d\n",UART_ERROR_received);	
				printf("ACTUATORS_received %d\n",ACTUATORS_received);	
				printf("\n");*/

				/*if(input_stream[3]==221){
					write_latency_to_netCDF(data->lisa_plane.baro_raw.tv);
					write_period_to_netCDF(data->lisa_plane.baro_raw.tv);
					int i;
					printf("Baro_raw content:");	
					print_mem((void *)&data->lisa_plane.baro_raw,sizeof(Baro_raw));
	
					printf("abs %d\n",data->lisa_plane.baro_raw.abs);
					printf("diff %d\n",data->lisa_plane.baro_raw.diff);	
					printf("\n");
					
					//print_latency(data->lisa_plane.baro_raw.tv);
									 
					printf("\n\n\n");
				}*/
				

				/*if(input_stream[3]==203){
					int i;
					printf("Imu_gyro_raw content:");
					print_mem((void *)&data->lisa_plane.imu_gyro_raw,sizeof(Imu_gyro_raw));
				
					printf("\n");
					printf("gp %d\n",data->lisa_plane.imu_gyro_raw.gp);
					printf("gq %d\n",data->lisa_plane.imu_gyro_raw.gq);
					printf("gr %d\n",data->lisa_plane.imu_gyro_raw.gr);
					
					print_latency(data->lisa_plane.imu_gyro_raw.tv);
				 
					printf("\n\n\n");

				}*/
				
				if(input_stream[3]==IMU_ACCEL_RAW){
					//write_latency_to_netCDF(data->lisa_plane.imu_accel_raw.tv);
					write_period_to_netCDF(data->lisa_plane.imu_accel_raw.tv);
					
					/*int i;
					printf("Imu_accel_raw content:");
					print_mem((void *)&data->lisa_plane.imu_accel_raw,sizeof(Imu_accel_raw));


					printf("\n");
					printf("ax %d\n",data->lisa_plane.imu_accel_raw.ax);
					printf("ay %d\n",data->lisa_plane.imu_accel_raw.ay);
					printf("az %d\n",data->lisa_plane.imu_accel_raw.az);
					printf("\n");*/
					
					//print_latency(data->lisa_plane.imu_accel_raw.tv);
			 
					//printf("\n\n\n");

				}
				
				/*if(input_stream[3]==57){
					int i;
					printf("airspeed content:");
					print_mem((void *)&data->lisa_plane.airspeed_ets,sizeof(Airspeed_ets));

					printf("\n");
					printf("adc %d\n",data->lisa_plane.airspeed_ets.adc);
					printf("offset %d\n",data->lisa_plane.airspeed_ets.offset);
					printf("scaled %f\n",data->lisa_plane.airspeed_ets.scaled);
					
					print_latency(data->lisa_plane.airspeed_ets.tv);


				}*/
				
				
				/*if(input_stream[3]==155){
					int i;
					printf("Gps_int_message content:");
					print_mem((void *)&data->lisa_plane.gps_int,sizeof(Gps_int));

					printf("\n");
					printf("ecef_x %d\n",data->lisa_plane.gps_int.ecef_x);
					printf("ecef_y %d\n",data->lisa_plane.gps_int.ecef_y);
					printf("ecef_z %d\n",data->lisa_plane.gps_int.ecef_z);
					printf("lat %d\n",data->lisa_plane.gps_int.lat);
					printf("lon %d\n",data->lisa_plane.gps_int.lon);
					printf("alt %d\n",data->lisa_plane.gps_int.alt);
					printf("hmsl %d\n",data->lisa_plane.gps_int.hmsl);
					printf("ecef_xd %d\n",data->lisa_plane.gps_int.ecef_xd);
					printf("ecef_yd %d\n",data->lisa_plane.gps_int.ecef_yd);
					printf("ecef_zd %d\n",data->lisa_plane.gps_int.ecef_zd);
					printf("pacc %d\n",data->lisa_plane.gps_int.pacc);
					printf("sacc %d\n",data->lisa_plane.gps_int.sacc);
					printf("tow %d\n",data->lisa_plane.gps_int.tow);
					printf("pdop %d\n",data->lisa_plane.gps_int.pdop);
					printf("numsv %d\n",data->lisa_plane.gps_int.numsv);
					printf("fix %d\n",data->lisa_plane.gps_int.fix);
					print_latency(data->lisa_plane.gps_int.tv);

					
				}*/
				
		
				/*if(input_stream[3]==33){
					int i;
					printf("sysmon content:");
					print_mem((void *)&data->lisa_plane.sys_mon,sizeof(Sys_mon));

				
					printf("\n");
					printf("periodic_time %d\n",data->lisa_plane.sys_mon.periodic_time);
					printf("periodic_cycle %d\n",data->lisa_plane.sys_mon.periodic_cycle);
					printf("periodic_cycle_min %d\n",data->lisa_plane.sys_mon.periodic_cycle_min);
					printf("periodic_cycle_max %d\n",data->lisa_plane.sys_mon.periodic_cycle_max);
					printf("event_number %d\n",data->lisa_plane.sys_mon.event_number);
					printf("cpu_load %d\n",data->lisa_plane.sys_mon.cpu_load);
					print_latency(data->lisa_plane.sys_mon.tv);

				}*/
			/*if(input_stream[3]==208){
					int i;
					printf("uart error content:");
					print_mem((void *)&data->lisa_plane.uart_errors,sizeof(UART_errors));

					printf("overrun_cnt %d\n",data->lisa_plane.uart_errors.overrun_cnt);
					printf("noise_err_cnt %d\n",data->lisa_plane.uart_errors.noise_err_cnt);
					printf("framing_err_cnt %d\n",data->lisa_plane.uart_errors.framing_err_cnt);
					printf("bus_number %d\n",data->lisa_plane.uart_errors.bus_number);
					print_latency(data->lisa_plane.uart_errors.tv);

				}*/
				/*if(input_stream[3]==105){
					int i;
					printf("actuators content:");
					print_mem((void *)&data->lisa_plane.actuators,sizeof(Actuators));

					printf("arr_length %d\n",data->lisa_plane.actuators.arr_length);
					for(i=0;i<data->lisa_plane.actuators.arr_length;i++){
						printf("servo_%d %d\n",i,data->lisa_plane.actuators.values[i]);
					}
					print_latency(data->lisa_plane.actuators.tv);

				}*/
							
			}else{
					printf("UNKNOW PACKAGE with id %d\n",input_stream[3]);
					exit(1);
			}
		}
	
		
	}

	UDP_err_handler(closeUDPServerSocket(&udp_server));

	/*------------------------END OF FIRST THREAD------------------------*/

	//wait for the second thread to finish
	if(pthread_join(thread_server_to_planebone, NULL)) {
		error_write(FILENAME,"error joining thread_lisa_to_pc");
		exit(EXIT_FAILURE);
	}

	return 0;
}

void *server_to_planebone(void *connection){
/*-------------------------START OF SECOND THREAD: SERVER TO PLANEBONE------------------------*/		
	
	Connection *conn=(Connection *)connection;
	static UDP udp_client;
	
	UDP_err_handler(openUDPClientSocket(&udp_client,conn->planebone_ip,conn->port_number_server_to_planebone,UDP_SOCKET_TIMEOUT));
	int i=0;
	
	while(1)
	{
		//1. read data from i don't now where
		uint8_t encoded_data[MAX_OUTPUT_STREAM_SIZE];
		Output output;

		//create test data
		output.message.servo_1=-i;
		output.message.servo_2=i;
		output.message.servo_3=i;
		output.message.servo_4=i;
		output.message.servo_5=i;
		output.message.servo_6=i;
		output.message.servo_7=0;
		i=i+600;
		if(i>12800){
			i=0;	
		}
	
		//2. encode the data	
		DEC_err_handler(data_encode(output.raw,sizeof(output.raw),encoded_data,1,72));
	
		/*printf("OUTPUT RAW:");
		int j;
			for(j=0;j<encoded_data[1];j++){
				printf("%d ",encoded_data[j]);
			}
		printf("\n");*/

		//3. send data to eth port using UDP
		UDP_err_handler(sendUDPClientData(&udp_client,&encoded_data,sizeof(encoded_data)));	
		
		//usleep(100000);
		sleep(1);
	}
	UDP_err_handler(closeUDPClientSocket(&udp_client));



	return NULL;
/*------------------------END OF SECOND THREAD------------------------*/
}	

static void UDP_err_handler( UDP_errCode err )  
{
	static char SOURCEFILE[] = "udp_communication.c";

	//it makes no sence to send UDP errors to server when there is a UDP problem.
	//write error to local log
	switch( err ) {
		case UDP_ERR_NONE:
			break;
		case  UDP_ERR_INET_ATON:
			error_write(SOURCEFILE,"failed decoding ip address");
			exit(EXIT_FAILURE);
			break;
		case UDP_ERR_SEND:
			error_write(SOURCEFILE,"failed sending UDP data");
			break;
		case UDP_ERR_CLOSE_SOCKET:
			error_write(SOURCEFILE,"failed closing UDP socket");
			break;
		case UDP_ERR_OPEN_SOCKET:
			error_write(SOURCEFILE,"failed inserting UDP socket");
			exit(EXIT_FAILURE);
			break;
		case UDP_ERR_BIND_SOCKET_PORT:
			error_write(SOURCEFILE,"failed binding port to socket");
			break;
		case UDP_ERR_RECV:
			error_write(SOURCEFILE,"failed receiving UDP data: timeout");
			break;
		case UDP_ERR_SET_TIMEOUT:
		error_write(SOURCEFILE,"failed setting UDP timeout on socket");
		break;
		case UDP_ERR_UNDEFINED:
			error_write(SOURCEFILE,"undefined UDP error");
			break;
		default: break;
	
	}
}

static void DEC_err_handler(DEC_errCode err )  
{
	static char SOURCEFILE[] = "data_decoding.c";
	//write error to local log
	switch( err ) {
		case DEC_ERR_NONE:
			break;
		case  DEC_ERR_START_BYTE:
			error_write(SOURCEFILE,"start byte is not 0x99");
			break;
		case DEC_ERR_CHECKSUM:
			error_write(SOURCEFILE,"wrong checksum");
			break;
		case DEC_ERR_UNKNOWN_BONE_PACKAGE:
			error_write(SOURCEFILE,"received unknown package from beaglebone");
			break;
		case DEC_ERR_UNKNOWN_LISA_PACKAGE:
			error_write(SOURCEFILE,"received unknown package from lisa");
			break;
		case DEC_ERR_UNKNOWN_SENDER:
			error_write(SOURCEFILE,"received package from unknown sender");
			break;
		case DEC_ERR_LENGTH:
			error_write(SOURCEFILE,"decoded not entire package length");
			break;
		case DEC_ERR_UNDEFINED:
			error_write(SOURCEFILE,"undefined decoding error");
			break;
		default: break;
	}
}

void print_mem(void const *vp, int n)
{
    unsigned char const *p = vp;
	int i;
	
    for (i=0; i<n; i++)
        printf("%d ", p[i]);
    printf("\n");
};

void print_latency(timeval tvSend){
	
		char strTime[64]={0};				
		timeval tvNow;
		timeval tvDiff;	
		
		gettimeofday(&tvNow, NULL);
						
		timestamp_to_timeString(tvSend,strTime);
		printf("send time \t%s\n",strTime);
		memset(strTime,0,sizeof(strTime));
					
		timestamp_to_timeString(tvNow,strTime);	
		printf("receive time \t%s\n",strTime);
					
		timeval_subtract(&tvDiff, &tvNow, &tvSend);	
		printf("latency %ld.%06ld sec\n", tvDiff.tv_sec, tvDiff.tv_usec);
}



