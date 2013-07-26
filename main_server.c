#include <stdio.h>

#include "udp_communication.h"
#include "communication_datatypes.h"


static UDP_server udp_server;
 

int main(int argc, char *argv[]){

	//create test struct to send test data
	Barometer barometer;
	Lisa_message lisa_message;

	openUDPServerSocket(&udp_server,8888);


	while(1){
	 	printf("\nWaiting for data...\n");
		fflush(stdout);

		receiveUDPServerData(&udp_server,(void *)&lisa_message,sizeof(lisa_message));
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
	}
	
	closeUDPServerSocket(&udp_server);
	
	
return 0;	
}

