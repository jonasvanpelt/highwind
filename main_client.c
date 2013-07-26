#include <stdio.h>

#include "udp_communication.h"
#include "communication_datatypes.h"
#include<stdlib.h> //exit(0);


static UDP_client udp_client;
 

int main(int argc, char *argv[]){
	
	int port_number;
	char *ip_address;
	
	if(argc == 3){
		//first argument is always name of program or empty string
		ip_address=argv[1];
		port_number=atoi(argv[2]);		
	}else{
			printf("wrong parameters: enter destination ip adress and port number\n");
			exit(1);
	}
	
//create test struct to send test data
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

	openUDPClientSocket(&udp_client,ip_address,port_number);
	sendUDPClientData(&udp_client,&lisa_message,sizeof(lisa_message));
	closeUDPClientSocket(&udp_client);
	
return 0;	
}

