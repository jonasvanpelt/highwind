#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "communication_datatypes.h"
#include "udp_communication.h"
#include "uart_communication.h"
#include "log.h"

static char FILENAME[] = "main_full_communication.c";

typedef struct{
		int port_number_lisa_to_pc;
		int port_number_pc_to_lisa;
		char *server_ip;
} Connection;


void *lisa_to_pc(void *connection){
/*-------------------------START OF SECOND THREAD: LISA TO PC------------------------*/	

	Connection *conn=(Connection *)connection;
	static UDP udp_client;

	//read data from UART
	
	serial_stream=serial_port_new();
	
	if (serial_port_setup()==-1)
	{
		error_write(FILENAME,"lisa_to_pc()","Setup has failed, port couldn't be opened");
		exit(1);
	}
	
	openUDPClientSocket(&udp_client,conn->server_ip,conn->port_number_lisa_to_pc);

	while(1)
	{
		if(serial_input_check()==0){
			//send data to eth port using UDP
			sendUDPClientData(&udp_client,&serial_input.buffer,sizeof(serial_input.buffer));
		}
	}

	serial_port_close();
	serial_port_free();
	closeUDPClientSocket(&udp_client);
	
	//the function must return something - NULL will do 
	return NULL;
/*------------------------END OF SECOND THREAD------------------------*/

}

int main(int argc, char *argv[]){
	Connection connection;
		
	if(argc == 4){
		//first argument is always name of program or empty string
		connection.server_ip=argv[1];
		connection.port_number_lisa_to_pc=atoi(argv[2]);	
		connection.port_number_pc_to_lisa=atoi(argv[3]);
	}else{
			printf("wrong parameters: server ip - send port number - receive port number\n");
			exit(1);		
	}
		
	//this variable is our reference to the second thread
	pthread_t thread_lisa_to_pc;;

	//create a second thread which executes lisa_to_pc
	if(pthread_create(&thread_lisa_to_pc, NULL, lisa_to_pc,&connection)) {
		printf("Error creating thread\n");	
		exit(1);
	}	
	
	/*-------------------------START OF FIRST THREAD: PC TO LISA------------------------*/
	static UDP udp_server;

	openUDPServerSocket(&udp_server,connection.port_number_pc_to_lisa);

	while(1){

		//1. retreive UDP data form PC from ethernet port.
		
		receiveUDPServerData(&udp_server,(void *)&serial_output.buffer,sizeof(serial_output.buffer)); //blocking !!!
			
		//2. send data to Lisa through UART port.
		
		serial_port_write();
	}

	closeUDPServerSocket(&udp_server);

	
	/*------------------------END OF FIRST THREAD------------------------*/
	
	
	//wait for the second thread to finish
	if(pthread_join(thread_lisa_to_pc, NULL)) {
		printf("Error joining thread\n");
		exit(2);
	}
	
	return 0;
}
	



