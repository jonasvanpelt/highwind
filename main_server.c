#include <stdio.h>
#include <stdint.h>
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
	uint8_t result[255]; 

	openUDPServerSocket(&udp_server,port_number);

	while(1){
	 	printf("\nWaiting for data...\n");
		fflush(stdout);

		receiveUDPServerData(&udp_server,(void *)&result,sizeof(result)); //blocking !!!
		
		//print details of the client/peer and the data received
		printf("start: %x\n", result[0]);
		printf("length: %d\n", result[1]);
		printf("send id: %d\n", result[2]);
		printf("message id: %d\n", result[3]);

		
	}
	
	closeUDPServerSocket(&udp_server);
	
	
return 0;	
}

