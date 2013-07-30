#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "communication_datatypes.h"
#include "udp_communication.h"
#include "uart_communication.h"
#include "log.h"
#include "circular_buffer.h"

#define CBSIZE 64

/************************************
 * PROTOTYPES
 * **********************************/
 void *lisa_to_pc(void *connection);
 void *data_logging_lisa(void *);


 /***********************************
  * GLOBALS
  * *********************************/

static char FILENAME[] = "main_full_communication.c";

typedef struct{
		int port_number_lisa_to_pc;
		int port_number_pc_to_lisa;
		char *server_ip;
} Connection;

//log buffer for data from lisa
 CircularBuffer cb_data_lisa;

//log buffer for data from groundstation
 CircularBuffer cb_data_ground;

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
			exit(1);		
	}
	//init cirucal data log buffers
	 cbInit(&cb_data_lisa, CBSIZE);
	 cbInit(&cb_data_ground, CBSIZE);

	//open uart port
	serial_stream=serial_port_new();
	
		
	if (serial_port_setup()==-1)
	{
		error_write(FILENAME,"lisa_to_pc()","Setup has failed, port couldn't be opened");
		exit(EXIT_FAILURE);
	}

	//this variable is our reference to the second thread
	pthread_t thread_lisa_to_pc,thread_data_logging_lisa;

	//create a second thread which executes lisa_to_pc
	if(pthread_create(&thread_lisa_to_pc, NULL, lisa_to_pc,&connection)) {
		error_write(FILENAME,"main()","error creating lisa thread");
		exit(1);
	}	
	
	//create a third thread for data loggin
	if(pthread_create(&thread_data_logging_lisa, NULL, data_logging_lisa,NULL)) {
		error_write(FILENAME,"main()","error creating logging thread");
		exit(1);
	}
	
	/*-------------------------START OF FIRST THREAD: PC TO LISA------------------------*/
	static UDP udp_server;
	ElemType cb_elem = {0};


	openUDPServerSocket(&udp_server,connection.port_number_pc_to_lisa);

	while(1){

		//1. retreive UDP data form PC from ethernet port.
		
		receiveUDPServerData(&udp_server,(void *)&serial_output.buffer,sizeof(serial_output.buffer)); //blocking !!!
		//write the data to circual buffer for log thread
		memcpy (&cb_elem.value, &serial_output.buffer, sizeof(serial_output.buffer));	
		cbWrite(&cb_data_ground, &cb_elem);
		
		int i;
		printf("\n output buffer: ");
		for(i=0;i<6;i++){
			printf("%d ",serial_output.set_servo_buffer[i]);
		}
		printf("\n");

			
		//2. send data to Lisa through UART port.
		
		serial_port_write();
	}

	closeUDPServerSocket(&udp_server);

	
	/*------------------------END OF FIRST THREAD------------------------*/
	
	
	//wait for the second thread to finish
	if(pthread_join(thread_lisa_to_pc, NULL)) {
		error_write(FILENAME,"main()","error joining thread");
		exit(EXIT_FAILURE);
	}
	
	cbFree(&cb_data_lisa);
	cbFree(&cb_data_ground);
	
	return 0;
}
void *lisa_to_pc(void *connection){
/*-------------------------START OF SECOND THREAD: LISA TO PC------------------------*/	

	Connection *conn=(Connection *)connection;
	static UDP udp_client;
	ElemType cb_elem = {0};

	//read data from UART
	
	openUDPClientSocket(&udp_client,conn->server_ip,conn->port_number_lisa_to_pc);

	while(1)
	{
		if(serial_input_check()==0){
			//send data to eth port using UDP
			sendUDPClientData(&udp_client,&serial_input.buffer,sizeof(serial_input.buffer));
			//write the data to circual buffer for log thread
			 memcpy (&cb_elem.value, &serial_input.buffer, sizeof(serial_input.buffer));	
			cbWrite(&cb_data_lisa, &cb_elem);


		}
	}

	serial_port_close();
	serial_port_free();
	closeUDPClientSocket(&udp_client);
	
	//the function must return something - NULL will do 
	return NULL;
/*------------------------END OF SECOND THREAD------------------------*/

}	

void *data_logging_lisa(void *arg){
	
	ElemType cb_elem = {0};
	init_log();
	open_data_lisa_log();
	
	while(1){
		while (!cbIsEmpty(&cb_data_lisa)) {
			cbRead(&cb_data_lisa, &cb_elem);
			write_data_lisa_log(cb_elem.value);
		}
	}
	close_data_lisa_log();
	
}



