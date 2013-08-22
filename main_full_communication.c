/*
 * AUTHOR: Jonas Van Pelt
 */

#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "udp_communication.h"
#include "uart_communication.h"
#include "log.h"
#include "circular_buffer.h"
#include "data_decoding.h"

#ifndef LOGGING
#define LOGGING 1
#endif

#define CBSIZE 1024 * 16
#define OUTPUT_BUFFER 36
#define MAX_STREAM_SIZE 255
#define UDP_SOCKET_TIMEOUT 1000000000


/************************************
 * PROTOTYPES
 * **********************************/
void *lisa_to_pc(void *connection);
void *data_logging_lisa(void *);
void *data_logging_groundstation(void *arg);
static void UDP_err_handler( UDP_errCode err,int exit_on_error );
static void UART_err_handler( UART_errCode err );
static void sendError(DEC_errCode err,library lib);
static void LOG_err_handler( LOG_errCode err );
static void switch_cb_lisa_pointers();
static void switch_cb_ground_pointers();

 /***********************************
  * GLOBALS
  * *********************************/

static char FILENAME[] = "main_full_communication.c";

typedef struct{
		int port_number_lisa_to_pc;
		int port_number_pc_to_lisa;
		char *server_ip;
} Connection;

static Connection connection;


#if LOGGING > 0
//log buffer for data from lisa
static CircularBuffer cb_data_lisa_ping;
static CircularBuffer cb_data_lisa_pong;
static CircularBuffer *cb_read_lisa = &cb_data_lisa_ping;
static CircularBuffer *cb_write_lisa = &cb_data_lisa_pong;
static int reading_flag_lisa=0;

//log buffer for data from groundstation
static CircularBuffer cb_data_ground_ping;
static CircularBuffer cb_data_ground_pong;
static CircularBuffer *cb_read_ground = &cb_data_ground_ping;
static CircularBuffer *cb_write_ground = &cb_data_ground_pong;
static int reading_flag_ground=0;


#endif

 /***********************************
  * MAIN
  * *********************************/
int main(int argc, char *argv[]){

	//parse arguments
	if(argc == 4){
		//first argument is always name of program or empty string
		connection.server_ip=argv[1];
		connection.port_number_lisa_to_pc=atoi(argv[2]);
		connection.port_number_pc_to_lisa=atoi(argv[3]);
	}else{
			printf("wrong parameters: server ip - send port number - receive port number\n");
			exit(EXIT_FAILURE);
	}

	//init log (mount sd card if necessary)

	int err = init_log();
	LOG_err_handler(err);

	if(err != LOG_ERR_NONE){
		exit(EXIT_FAILURE);		//mounting SD card failed
	}

	#if LOGGING > 0

	//init circular data log buffers
	 cbInit(cb_read_lisa, CBSIZE);
	 cbInit(cb_write_lisa, CBSIZE);
	 cbInit(cb_read_ground, CBSIZE);
	 cbInit(cb_write_ground, CBSIZE);


	 #endif

	//open uart port
	serial_stream=serial_port_new();

	UART_err_handler(serial_port_setup());

	//thread variables
	pthread_t thread_lisa_to_pc,thread_data_logging_lisa,thread_data_logging_ground;

	//create a second thread which executes lisa_to_pc
	if(pthread_create(&thread_lisa_to_pc, NULL, lisa_to_pc,&connection)) {
		error_write(FILENAME,"error creating lisa thread");
		exit(EXIT_FAILURE);
	}

	#if LOGGING > 0

	//create a third thread which executes data_logging_lisa
	if(pthread_create(&thread_data_logging_lisa, NULL, data_logging_lisa,NULL)) {
		error_write(FILENAME,"error creating lisa logging thread");
		exit(EXIT_FAILURE);
	}

	//create a fourth thread which executes data_logging_groundstation
	if(pthread_create(&thread_data_logging_ground, NULL, data_logging_groundstation,NULL)) {
		error_write(FILENAME,"error creating groundstation logging thread");
		exit(EXIT_FAILURE);
	}

	#endif
	/*-------------------------START OF FIRST THREAD: PC TO LISA------------------------*/
	static UDP udp_server;
	uint8_t input_stream[OUTPUT_BUFFER];

	ElemType cb_elem = {0};

	//init the data decode pointers
	init_decoding();

	UDP_err_handler(openUDPServerSocket(&udp_server,connection.port_number_pc_to_lisa,UDP_SOCKET_TIMEOUT),1);

	while(1){

		//1. retreive UDP data form PC from ethernet port.
		err=receiveUDPServerData(&udp_server,(void *)&input_stream,sizeof(input_stream)); //blocking !!!
		UDP_err_handler(err,0);

		if(err==UDP_ERR_NONE){

			/*printf("INCOMING OUTPUT RAW:");
			int j;
			for(j=0;j<input_stream[1];j++){
				printf("%d ",input_stream[j]);
			}
			printf("\n");*/

			#if LOGGING > 0
			
			if(!cbIsFull(cb_write_ground)){
				 memcpy (&cb_elem.value, &input_stream, sizeof(input_stream));
				 cbWrite(cb_write_ground, &cb_elem);
			 }else{
				if(reading_flag_ground==0){
					switch_cb_ground_pointers();
					printf("switching ground pointers\n");
				}else{
					printf("GROUND WRITE WAS NOT READY \n");
					exit(1); //FOR DEBUGGING
				}
			 }
			
			#endif

			int new_length = strip_timestamp(input_stream); //lisa expects a package without a timestamp

			UART_err_handler(serial_port_write(input_stream,new_length));
		}

	}
	serial_port_close();
	UDP_err_handler(closeUDPServerSocket(&udp_server),0);
	/*------------------------END OF FIRST THREAD------------------------*/


	//wait for the second thread to finish
	if(pthread_join(thread_lisa_to_pc, NULL)) {
		error_write(FILENAME,"error joining thread_lisa_to_pc");
	}

	#if LOGGING > 0

	//wait for the third thread to finish
	if(pthread_join(thread_data_logging_lisa, NULL)) {
		error_write(FILENAME,"error joining thread_data_logging_lisa");
	}


	//wait for the fourth thread to finish
	if(pthread_join(thread_data_logging_ground, NULL)) {
		error_write(FILENAME,"error joining thread_data_logging_ground");
	}

	//free circular buffers
	cbFree(cb_read_lisa);
	cbFree(cb_write_lisa);
	cbFree(cb_read_ground);
	cbFree(cb_write_ground);

	#endif

	return 0;
}

/************************************
 * FUNCTIONS
 * **********************************/
void *lisa_to_pc(void *arg){
/*-------------------------START OF SECOND THREAD: LISA TO PC------------------------*/

	static UDP udp_client;
	int message_length;
	ElemType cb_elem = {0};

	//read data from UART

	UDP_err_handler(openUDPClientSocket(&udp_client,connection.server_ip,connection.port_number_lisa_to_pc,UDP_SOCKET_TIMEOUT),1);


	while(1)
	{
		message_length = serial_input_check(); //blocking !!!
		if(message_length !=UART_ERR_READ){

			//add timestamp
			message_length=add_timestamp(serial_input.buffer);

			//send data to eth port using UDP
			UDP_err_handler(sendUDPClientData(&udp_client,&(serial_input.buffer),message_length),0);

			#if LOGGING > 0

			//write the data to circual buffer for log thread
			 if(!cbIsFull(cb_write_lisa)){
				 memcpy (&cb_elem.value, &(serial_input.buffer), message_length);
				 cbWrite(cb_write_lisa, &cb_elem);
			 }else{
				if(reading_flag_lisa==0){
					switch_cb_lisa_pointers();
					printf("switching lisa pointers\n");
				}else{
					printf("LISA WRITE WAS NOT READY \n");
					exit(1); //FOR DEBUGGING
				}
			 }

			#endif
		}else{
		//send error message to server: not receiving data on uart port
			printf("no data on uart\n");
			UART_err_handler(message_length);
		}

	}

	serial_port_close();
	serial_port_free();
	UDP_err_handler(closeUDPClientSocket(&udp_client),0);

	return NULL;
/*------------------------END OF SECOND THREAD------------------------*/

}

#if LOGGING > 0

void *data_logging_lisa(void *arg){
/*-------------------------START OF THIRD THREAD: LISA TO PC LOGGING------------------------*/

	ElemType cb_elem = {0};
	LOG_err_handler(open_data_lisa_log());

	while(1){
		if (!cbIsEmpty(cb_read_lisa)) {
			reading_flag_lisa=1;
			cbRead(cb_read_lisa, &cb_elem);
			LOG_err_handler(write_data_lisa_log(cb_elem.value,cb_elem.value[1]));
			usleep(100);
		}else{
			reading_flag_lisa=0;
			usleep(1000);
		}	
	}
	LOG_err_handler(close_data_lisa_log());

	return NULL;
/*-------------------------END OF THIRD THREAD: LISA TO PC LOGGING------------------------*/
}

void *data_logging_groundstation(void *arg){
/*-------------------------START OF FOURTH THREAD: GROUNDSTATION TO LISA LOGGING------------------------*/

	ElemType cb_elem = {0};
	LOG_err_handler(open_data_groundstation_log());
	
	while(1){
		if (!cbIsEmpty(cb_read_ground)) {
			reading_flag_ground=1;
			cbRead(cb_read_ground, &cb_elem);
			LOG_err_handler(write_data_groundstation_log(cb_elem.value,cb_elem.value[1]));
			usleep(100);
		}else{
			reading_flag_ground=0;
			usleep(1000);
		}	
	}

	LOG_err_handler(close_data_groundstation_log());

	return NULL;
/*-------------------------END OF FOURTH THREAD: GROUNDSTATION TO LISA LOGGING------------------------*/

}

#endif


static void UDP_err_handler( UDP_errCode err,int exit_on_error)
{
	static char SOURCEFILE[] = "udp_communication.c";

	//it makes no sence to send UDP errors to server when there is a UDP problem.
	//write error to local log
	switch( err ) {
		case UDP_ERR_NONE:
			break;
		case  UDP_ERR_INET_ATON:
			error_write(SOURCEFILE,"failed decoding ip address");
			break;
		case UDP_ERR_SEND:
			error_write(SOURCEFILE,"failed sending UDP data");
			break;
		case UDP_ERR_CLOSE_SOCKET:
			error_write(SOURCEFILE,"failed closing UDP socket");
			break;
		case UDP_ERR_OPEN_SOCKET:
			error_write(SOURCEFILE,"failed opening UDP socket");
			break;
		case UDP_ERR_BIND_SOCKET_PORT:
			error_write(SOURCEFILE,"failed binding port to socket");
			break;
		case UDP_ERR_RECV:
			error_write(SOURCEFILE,"failed receiving UDP data");
			break;
		case UDP_ERR_SET_TIMEOUT:
			error_write(SOURCEFILE,"failed setting UDP timeout on socket");
			break;
		case UDP_ERR_UNDEFINED:
			error_write(SOURCEFILE,"undefined UDP error");
			break;
		default: break;
	}
	if(exit_on_error && err != UDP_ERR_NONE)
		exit(EXIT_FAILURE);
}

static void UART_err_handler( UART_errCode err )
{
	static char SOURCEFILE[] = "uart_communication.c";

	//write error to local log
	switch( err ) {
			case UART_ERR_NONE:

				break;
			case  UART_ERR_READ:
				error_write(SOURCEFILE,"failed reading data from UART");
				break;
			case UART_ERR_SERIAL_PORT_FLUSH_INPUT:
				error_write(SOURCEFILE,"serial port flush input failed");
				break;
			case UART_ERR_SERIAL_PORT_FLUSH_OUTPUT:
				error_write(SOURCEFILE,"serial port flush output failed");
				break;
			case UART_ERR_SERIAL_PORT_OPEN:
				error_write(SOURCEFILE,"serial port open failed");
				exit(EXIT_FAILURE);
				break;
			case UART_ERR_SERIAL_PORT_CLOSE:
				error_write(SOURCEFILE,"serial port close failed");
				break;
			case UART_ERR_SERIAL_PORT_CREATE:
				error_write(SOURCEFILE,"serial port create failed");
				exit(EXIT_FAILURE);
				break;
			case UART_ERR_SERIAL_PORT_WRITE:
				error_write(SOURCEFILE,"serial port write failed");
				break;
			case UART_ERR_UNDEFINED:
				error_write(SOURCEFILE,"undefined UART error");
				break;
			default: break;
		}

	if(err != UART_ERR_NONE){
		sendError(err,UART_L);
	}
}

static void LOG_err_handler( LOG_errCode err )
{
	//send error to server
	if(err != LOG_ERR_NONE){
		sendError(err,LOG_L);
	}

}

static void sendError(DEC_errCode err,library lib){

		static UDP udp_client;
		int message_length;
		char encoded_data[MAX_STREAM_SIZE];
		Data data;
		Beagle_error error_message;

		//encode an error package
		error_message.message.library=lib;
		error_message.message.error=err;
		data_encode(error_message.raw,sizeof(error_message.raw),encoded_data,2,2);
		message_length=sizeof(encoded_data);

		//send errorcode to server
		UDP_err_handler(openUDPClientSocket(&udp_client,connection.server_ip,connection.port_number_lisa_to_pc,UDP_SOCKET_TIMEOUT),0);
		UDP_err_handler(sendUDPClientData(&udp_client,&encoded_data,message_length),0);
		UDP_err_handler(closeUDPClientSocket(&udp_client),0);
}

static void switch_cb_lisa_pointers(){
		CircularBuffer *temp = cb_read_lisa;
		cb_read_lisa = cb_write_lisa;
		cb_write_lisa = temp;
}

static void switch_cb_ground_pointers(){
		CircularBuffer *temp = cb_read_ground;
		cb_read_ground = cb_write_ground;
		cb_write_ground = temp;
}
