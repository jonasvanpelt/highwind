/*
 * AUTHOR: Jonas Van Pelt
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "udp_communication.h"
#include "uart_communication.h"
#include "log.h"
#include "circular_buffer.h"
#include "data_decoding.h"

#ifndef LOGGING 
#define LOGGING 0
#endif


#define CBSIZE 2048
#define OUTPUT_BUFFER 34
#define MAX_STREAM_SIZE 255


/************************************
 * PROTOTYPES
 * **********************************/
 void *lisa_to_pc(void *connection);
 void *data_logging_lisa(void *);
 void *data_logging_groundstation(void *arg);
 static void UDP_err_handler( UDP_errCode err ); 
 static void UART_err_handler( UART_errCode err );   
 static void Decode_err_handler( UART_errCode err ); 


 /***********************************
  * GLOBALS
  * *********************************/

static char FILENAME[] = "main_full_communication.c";

typedef struct{
		int port_number_lisa_to_pc;
		int port_number_pc_to_lisa;
		int port_number_error_message;
		char *server_ip;
} Connection;

Connection connection;


#if LOGGING > 0
//log buffer for data from lisa
 CircularBuffer cb_data_lisa;

//log buffer for data from groundstation
 CircularBuffer cb_data_ground;
 
#endif

 /***********************************
  * MAIN
  * *********************************/
int main(int argc, char *argv[]){
	
	//parse arguments	
	if(argc == 5){
		//first argument is always name of program or empty string
		connection.server_ip=argv[1];
		connection.port_number_lisa_to_pc=atoi(argv[2]);	
		connection.port_number_pc_to_lisa=atoi(argv[3]);
		connection.port_number_error_message=atoi(argv[4]);
	}else{
			printf("wrong parameters: server ip - send port number - receive port number - error message port number\n");
			exit(EXIT_FAILURE);		
	}
	
	//init log (mount sd card if necessary)
	if(init_log()==-1){
		error_write(FILENAME,"main()","Init log failed");
		exit(EXIT_FAILURE);
	}
	
	#if LOGGING > 0

	//init circular data log buffers
	 cbInit(&cb_data_lisa, CBSIZE);
	 cbInit(&cb_data_ground, CBSIZE);
	 
	 #endif

	//open uart port
	serial_stream=serial_port_new();
	
	UART_err_handler(serial_port_setup());

	pthread_t thread_lisa_to_pc,thread_data_logging_lisa,thread_data_logging_ground;

	//create a second thread which executes lisa_to_pc
	if(pthread_create(&thread_lisa_to_pc, NULL, lisa_to_pc,&connection)) {
		error_write(FILENAME,"main()","error creating lisa thread");
		exit(EXIT_FAILURE);
	}	
	
	#if LOGGING > 0 

	//create a third thread which executes data_logging_lisa
	if(pthread_create(&thread_data_logging_lisa, NULL, data_logging_lisa,NULL)) {
		error_write(FILENAME,"main()","error creating lisa logging thread");
		exit(EXIT_FAILURE);
	}
	
	//create a fourth thread which executes data_logging_groundstation
	if(pthread_create(&thread_data_logging_ground, NULL, data_logging_groundstation,NULL)) {
		error_write(FILENAME,"main()","error creating groundstation logging thread");
		exit(EXIT_FAILURE);
	}
	
	#endif
	/*-------------------------START OF FIRST THREAD: PC TO LISA------------------------*/
	static UDP udp_server;
	uint8_t input_stream[OUTPUT_BUFFER];

	ElemType cb_elem = {0};
	
	//init the data decode pointers
	init_decoding();

	UDP_err_handler(openUDPServerSocket(&udp_server,connection.port_number_pc_to_lisa));

	while(1){

		//1. retreive UDP data form PC from ethernet port.
		
		UDP_err_handler(receiveUDPServerData(&udp_server,(void *)&input_stream,sizeof(input_stream))); //blocking !!!
		UART_err_handler(serial_port_write(input_stream)); 
		
		#if LOGGING > 0
		
		//write the data to circular buffer for log thread
		memcpy (&cb_elem.value, &input_stream, sizeof(input_stream));	
		cbWrite(&cb_data_ground, &cb_elem);
			
		//FOR DEBUGGING: REMOVE ME!!!
		if(cbIsFull(&cb_data_ground)){
			printf("groundstation buffer is full\n") ;
		}
		
		#endif

	}
	
	serial_port_close();
	UDP_err_handler(closeUDPServerSocket(&udp_server));
	/*------------------------END OF FIRST THREAD------------------------*/
	
	
	//wait for the second thread to finish
	if(pthread_join(thread_lisa_to_pc, NULL)) {
		error_write(FILENAME,"main()","error joining thread_lisa_to_pc");
		exit(EXIT_FAILURE);
	}
	
	#if LOGGING > 0 
	
	//wait for the third thread to finish
	if(pthread_join(thread_data_logging_lisa, NULL)) {
		error_write(FILENAME,"main()","error joining thread_data_logging_lisa");
		exit(EXIT_FAILURE);
	}
	
	
	//wait for the fourth thread to finish
	if(pthread_join(thread_data_logging_ground, NULL)) {
		error_write(FILENAME,"main()","error joining thread_data_logging_ground");
		exit(EXIT_FAILURE);
	}
	
	
	//free circular buffers
	cbFree(&cb_data_lisa);
	cbFree(&cb_data_ground);
	
	#endif
	
	return 0;
}
void *lisa_to_pc(void *arg){
/*-------------------------START OF SECOND THREAD: LISA TO PC------------------------*/	

	static UDP udp_client;
	int message_length;
	ElemType cb_elem = {0};

	//read data from UART
	
	UDP_err_handler(openUDPClientSocket(&udp_client,connection.server_ip,connection.port_number_lisa_to_pc));

	while(1)
	{
		message_length = serial_input_check();
		if(message_length !=UART_ERR_READ){
		
			//send data to eth port using UDP
			UDP_err_handler(sendUDPClientData(&udp_client,&(serial_input.buffer),message_length));
	
			#if LOGGING > 0

			//write the data to circual buffer for log thread
			 memcpy (&cb_elem.value, &(serial_input.buffer), message_length);	
			 cbWrite(&cb_data_lisa, &cb_elem);
			 
			 //FOR DEBUGGING: REMOVE ME!!!
			 if(cbIsFull(&cb_data_lisa)){
				printf("lisa buffer is full\n") ;
			}
			
			#endif
		}
	}
	serial_port_close();
	serial_port_free();
	UDP_err_handler(closeUDPClientSocket(&udp_client));
	
	//the function must return something - NULL will do 
	return NULL;
/*------------------------END OF SECOND THREAD------------------------*/

}	

#if LOGGING > 0

void *data_logging_lisa(void *arg){
/*-------------------------START OF THIRD THREAD: LISA TO PC LOGGING------------------------*/	

	ElemType cb_elem = {0};
	open_data_lisa_log();
	
	while(1){
		while (!cbIsEmpty(&cb_data_lisa)) {
			cbRead(&cb_data_lisa, &cb_elem);
			write_data_lisa_log(cb_elem.value);
		}
		usleep(10);
	}
	close_data_lisa_log();
/*-------------------------END OF THIRD THREAD: LISA TO PC LOGGING------------------------*/	
}

void *data_logging_groundstation(void *arg){
/*-------------------------START OF FOURTH THREAD: GROUNDSTATION TO LISA LOGGING------------------------*/	

	ElemType cb_elem = {0};
	open_data_groundstation_log();
	
	while(1){
		while (!cbIsEmpty(&cb_data_ground)) {
			cbRead(&cb_data_ground, &cb_elem);
			write_data_groundstation_log(cb_elem.value);
		}
		usleep(20);
	}
	close_data_groundstation_log();
/*-------------------------END OF FOURTH THREAD: GROUNDSTATION TO LISA LOGGING------------------------*/	

}

#endif

static void UDP_err_handler( UDP_errCode err )  
{
	//it makes no sence to send UDP errors to server when there is a UDP problem.
	//write error to local log
	switch( err ) {
		case UDP_ERR_NONE:
			break;
		case  UDP_ERR_INET_ATON:
			error_write(FILENAME,"main()","failed decoding ip address");
			exit(EXIT_FAILURE);
			break;
		case UDP_ERR_SEND:
			error_write(FILENAME,"main()","failed sending UDP data");
			break;
		case UDP_ERR_CLOSE_SOCKET:
			error_write(FILENAME,"main()","failed closing UDP socket");
			break;
		case UDP_ERR_OPEN_SOCKET:
			error_write(FILENAME,"main()","failed inserting UDP socket");
			exit(EXIT_FAILURE);
			break;
		case UDP_ERR_BIND_SOCKET_PORT:
			error_write(FILENAME,"main()","failed binding port to socket");
			break;
		case UDP_ERR_RECV:
			error_write(FILENAME,"main()","failed receiving UDP data");
			break;
		case UDP_ERR_UNDEFINED:
			error_write(FILENAME,"main()","undefined UDP error");
			break;
		default: break;// should never come here
	
	}
}

static void UART_err_handler( UART_errCode err )  
{
	//write error to local log
	switch( err ) {
			case UART_ERR_NONE:
				break;
			case  UART_ERR_READ:
				error_write(FILENAME,"main()","failed reading data from UART");
				break;
			case UART_ERR_SERIAL_PORT_FLUSH_INPUT:
				error_write(FILENAME,"main()","serial port flush input failed");
				break;
			case UART_ERR_SERIAL_PORT_FLUSH_OUTPUT:
				error_write(FILENAME,"main()","serial port flush output failed");
				break;
			case UART_ERR_SERIAL_PORT_OPEN:
				error_write(FILENAME,"main()","serial port open failed");
				exit(EXIT_FAILURE);
				break;
			case UART_ERR_SERIAL_PORT_CLOSE:
				error_write(FILENAME,"main()","serial port close failed");
				break;
			case UART_ERR_SERIAL_PORT_CREATE:
				error_write(FILENAME,"main()","serial port create failed");
				exit(EXIT_FAILURE);
				break;
			case UART_ERR_SERIAL_PORT_WRITE:
				error_write(FILENAME,"main()","serial port write failed");
				break;
			case UART_ERR_UNDEFINED:
				error_write(FILENAME,"main()","undefined UART error");
				break;
			default: break;// should never come here
		
		}
	
	if(!UART_ERR_NONE){
		static UDP udp_client;
		int message_length;
		char encoded_data[MAX_STREAM_SIZE];
		Data data;
		Error error_message;

		//encode an error package
		error_message.message.library=UART_L;
		error_message.message.error=err;
		data_encode(error_message.raw,encoded_data,2,2);
		
		//send errorcode to server
		UDP_err_handler(openUDPClientSocket(&udp_client,connection.server_ip,connection.port_number_error_message));
		UDP_err_handler(sendUDPClientData(&udp_client,&encoded_data,message_length));
		UDP_err_handler(closeUDPClientSocket(&udp_client));

	}
	
}

/*static void Decode_err_handler( UART_errCode err )  
{
	//send errorcode to server
}
*/
