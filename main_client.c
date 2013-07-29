#include <stdio.h>

#include "udp_communication.h"
#include "communication_datatypes.h"
#include<stdlib.h> //exit(0);


static UDP udp_client;
 

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
	
	int test = 100;

	openUDPClientSocket(&udp_client,ip_address,port_number);
	sendUDPClientData(&udp_client,&test,sizeof(test));
	closeUDPClientSocket(&udp_client);
	
return 0;	
}

