#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "communication_datatypes.h"
#include "udp_communication.h"

typedef struct{
		int port_number_lisa_to_pc;
		int port_number_pc_to_lisa;
		char *server_ip;
} Connection;


void *lisa_to_pc(void *connection){
/*-------------------------START OF SECOND THREAD: LISA TO PC------------------------*/	

	Connection *conn=(Connection *)connection;

	//read data from UART
	
	//send data to eth port using UDP
	static UDP_client udp_client;

	Barometer barometer;
	Lisa_message lisa_message;
	
	barometer.abs=100;
	barometer.diff=5;


	lisa_message.start=0x99;
	lisa_message.length=10;
	lisa_message.aircraft_id=1;
	lisa_message.checksum_A=10;
	lisa_message.checksum_B=20;
	lisa_message.barometer=barometer;

	openUDPClientSocket(&udp_client,conn->server_ip,conn->port_number_lisa_to_pc);
	sendUDPClientData(&udp_client,&lisa_message,sizeof(lisa_message));
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
	Barometer barometer;
	Lisa_message lisa_message;
	
	openUDPServerSocket(&udp_server,connection.port_number_pc_to_lisa);

	while(1){

		//1. retreive UDP data form PC from ethernet port.
		
		receiveUDPServerData(&udp_server,(void *)&lisa_message,sizeof(lisa_message)); //blocking !!!
		
		//print details of the client/peer and the data received

		printf("\nReceived packet from %s:%d\n", inet_ntoa(udp_server.si_other.sin_addr), ntohs(udp_server.si_other.sin_port));
		printf("Received structure:\n");
		printf("start: %x\n" , lisa_message.start);
		printf("length: %d\n" , lisa_message.length);
		printf("aircraft_id: %d\n" , lisa_message.aircraft_id);
		printf("checksum_A: %d\n" , lisa_message.checksum_A);
		printf("checksum_B: %d\n" , lisa_message.checksum_B);
		printf("barometer abs: %d\n" , lisa_message.barometer.abs);
		printf("barometer diff: %d\n" , lisa_message.barometer.diff);
		
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
	



