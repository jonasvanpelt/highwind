#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "communication_datatypes.h"
#include "udp_communication.h"
#include "uart_communication.h"


typedef struct{
		int port_number_lisa_to_pc;
		int port_number_pc_to_lisa;
		char *server_ip;
} Connection;


void *lisa_to_pc(void *connection){
/*-------------------------START OF SECOND THREAD: LISA TO PC------------------------*/	

	Connection *conn=(Connection *)connection;
	static UDP_client udp_client;


	//read data from UART
	
	serial_stream=serial_port_new();
	packets_clear(); //for debugging

	if (!serial_port_setup())
	{
	#if DEBUG > 0
		printf("Setup has failed, port couldn't be opened\n");
	#endif
		exit(1);
	}
	
	openUDPClientSocket(&udp_client,conn->server_ip,conn->port_number_lisa_to_pc);


	while(1)
	{
		if(serial_input_check()==1){
			//send data to eth port using UDP
			sendUDPClientData(&udp_client,&serial_input.buffer,sizeof(serial_input.buffer));
			serial_input_buffer_clear();
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
			printf("wrong parameters\n");
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
	
	
	static UDP_server udp_server;
	union Serial_input {
		char buffer[14]; //must be set bigger
		struct Serial_input_conversion{
			uint8_t start;
			uint8_t length;
			uint8_t sender_id;
			uint8_t message_id;
			uint32_t baro_raw_abs;
			uint32_t baro_raw_diff;
			uint8_t checksum_1;
			uint8_t checksum_2;
		} converted;
	} result;
	
	openUDPServerSocket(&udp_server,connection.port_number_pc_to_lisa);

	while(1){

		//1. retreive UDP data form PC from ethernet port.
		
		receiveUDPServerData(&udp_server,(void *)&result,sizeof(result)); //blocking !!!
		
		//print details of the client/peer and the data received
		printf("start: %X ", result.converted.start);
		printf("length: %d ", result.converted.length);
		printf("checksum_1: %d ", result.converted.checksum_1);
		printf("checksum_2: %d ", result.converted.checksum_2);		
		
		//2. send data to Lisa through UART port.
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
	



