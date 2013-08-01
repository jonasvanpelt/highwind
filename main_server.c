#include <stdio.h>
#include<stdlib.h> //exit(0);

#include "udp_communication.h"
#include "communication_datatypes.h"


static UDP udp_server;
 

int main(int argc, char *argv[]){
	
	int port_number;
	
	if(argc == 2){
		//first argument is always name of program or empty string
		port_number=atoi(argv[1]);
	}else{
			printf("wrong parameters: enter port number\n");
			exit(1);		
	}

	//create test struct to send test data
	/*Barometer barometer;
	Lisa_message lisa_message;*/
	
	union Serial_input {
		char buffer[255]; //must be set bigger
	} result;

	openUDPServerSocket(&udp_server,port_number);

	while(1){
	 	printf("\nWaiting for data...\n");
		fflush(stdout);

		receiveUDPServerData(&udp_server,(void *)&result,sizeof(result)); //blocking !!!
		
		//print details of the client/peer and the data received
		printf("start: %x ", result.buffer[0]);
		printf("length: %d ", result.buffer[1]);
		printf("send id: %d ", result.buffer[2]);
		printf("message id: %d ", result.buffer[3]);

		
	}
	
	closeUDPServerSocket(&udp_server);
	
	
return 0;	
}

