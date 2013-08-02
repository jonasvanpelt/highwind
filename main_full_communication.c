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

#ifndef LOGGING 
#define LOGGING 1
#endif


#define CBSIZE 2048

#define MAX_STREAM_SIZE 255


/************************************
 * PROTOTYPES
 * **********************************/
 void *lisa_to_pc(void *connection);
 void *data_logging_lisa(void *);
 void *data_logging_groundstation(void *arg);


 /***********************************
  * GLOBALS
  * *********************************/

static char FILENAME[] = "main_full_communication.c";

typedef struct{
		int port_number_lisa_to_pc;
		int port_number_pc_to_lisa;
		char *server_ip;
} Connection;

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
	Connection connection;
	
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
		
	if (serial_port_setup()==-1)
	{
		error_write(FILENAME,"main()","Setup has failed, uart port couldn't be opened");
		exit(EXIT_FAILURE);
	}

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
	ElemType cb_elem = {0};

	openUDPServerSocket(&udp_server,connection.port_number_pc_to_lisa);

	while(1){

		//1. retreive UDP data form PC from ethernet port.
		
		receiveUDPServerData(&udp_server,(void *)&serial_output.buffer,sizeof(serial_output.buffer)); //blocking !!!
			
		//2. send data to Lisa through UART port.
		
		#if LOGGING > 0
		
		//write the data to circular buffer for log thread
		memcpy (&cb_elem.value, &serial_output.buffer, sizeof(serial_output.buffer));	
		cbWrite(&cb_data_ground, &cb_elem);
			
		//FOR DEBUGGING: REMOVE ME!!!
		if(cbIsFull(&cb_data_ground)){
			printf("groundstation buffer is full\n") ;
		}
			
		//FOR DEBUGGING: REMOVE ME!!!
		/*int i;
		printf("\n output buffer: ");
		for(i=0;i<6;i++){
			printf("%d ",serial_output.set_servo_buffer[i]);
		}
		printf("\n");*/
		
		#endif

	}

	closeUDPServerSocket(&udp_server);
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
void *lisa_to_pc(void *connection){
/*-------------------------START OF SECOND THREAD: LISA TO PC------------------------*/	

	Connection *conn=(Connection *)connection;
	static UDP udp_client;
	int message_length;
	ElemType cb_elem = {0};

	//read data from UART
	
	openUDPClientSocket(&udp_client,conn->server_ip,conn->port_number_lisa_to_pc);

	while(1)
	{
		message_length = serial_input_check();
		if(message_length !=-1){
		
			//send data to eth port using UDP
			sendUDPClientData(&udp_client,&(serial_input.buffer),message_length);
	
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
	closeUDPClientSocket(&udp_client);
	
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
		usleep(20);
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


