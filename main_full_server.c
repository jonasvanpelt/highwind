#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "udp_communication.h"
#include "data_decoding.h"
#include "log.h"

#define MAX_INPUT_STREAM_SIZE 255
#define MAX_OUTPUT_STREAM_SIZE 36

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

	UDP_err_handler(openUDPServerSocket(&udp_server,connection.port_number_planebone_to_server));
	
	//init the data decode pointers
	init_decoding();

	while(1){
		//printf("\nWaiting for data...\n\n");
		fflush(stdout);

		//1. retreive UDP data form planebone from ethernet port.
		
		UDP_err_handler(receiveUDPServerData(&udp_server,(void *)&input_stream,sizeof(input_stream))); //blocking !!!
		
		#if DEBUG > 0
		
		printf("data raw: ");
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

			if(input_stream[3]==221){

			int i;
			printf("Baro_raw content:");

			for(i=0;i<input_stream[1];i++){
				printf("%d ",data->lisa_plane.baro_raw.raw[i]);
			}
			printf("\n");
			printf("abs %d\n",data->lisa_plane.baro_raw.message.abs);
			printf("diff %d\n",data->lisa_plane.baro_raw.message.diff);
			}

			printf("\n");
			if(input_stream[3]==203){
				int i;
				printf("Imu_gyro_raw content:");

				for(i=0;i<input_stream[1];i++){
					printf("%d ",data->lisa_plane.imu_gyro_raw.raw[i]);
				}
				printf("\n");
				printf("gp %d\n",data->lisa_plane.imu_gyro_raw.message.gp);
				printf("gp %d\n",data->lisa_plane.imu_gyro_raw.message.gp);
				printf("gr %d\n",data->lisa_plane.imu_gyro_raw.message.gr);

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
	
	UDP_err_handler(openUDPClientSocket(&udp_client,conn->planebone_ip,conn->port_number_server_to_planebone));
	int i=0;
	
	while(1)
	{
		//1. read data from i do'nt now where
		
		
		uint8_t encoded_data[MAX_OUTPUT_STREAM_SIZE];
		Output output;

		//create test data

		output.message.servo_1=1000;
		output.message.servo_2=1000;
		output.message.servo_3=i;
		output.message.servo_4=i;
		output.message.servo_5=1000;
		output.message.servo_6=1000;
		
		if(i==500){
			i=5;
		}else{
			i=500;
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
		
		//sleep(1);
		usleep(100000);
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
			error_write(SOURCEFILE,"failed receiving UDP data");
			break;
		case UDP_ERR_UNDEFINED:
			error_write(SOURCEFILE,"undefined UDP error");
			break;
		default: break;// should never come here
	
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
			error_write(SOURCEFILE,"received package from unknown sender ");
			break;
		case DEC_ERR_UNDEFINED:
			error_write(SOURCEFILE,"undefined decoding error");
			break;
		default: break;// should never come here
	}
}




