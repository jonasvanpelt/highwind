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

	openUDPServerSocket(&udp_server,port_number);

	while(1){
	 	printf("\nWaiting for data...\n");
		fflush(stdout);

		receiveUDPServerData(&udp_server,(void *)&result,sizeof(result)); //blocking !!!
		
		//print details of the client/peer and the data received
		printf("start: %x ", result.converted.start);
		printf("length: %d ", result.converted.length);
		printf("checksum_1: %d ", result.converted.checksum_1);
		printf("checksum_2: %d ", result.converted.checksum_2);
		
	}
	
	closeUDPServerSocket(&udp_server);
	
	
return 0;	
}

