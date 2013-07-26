#include <stdio.h>

#include "udp_communication.h"
#include "communication_datatypes.h"


static UDP_client udp_client;
 

int main(int argc, char *argv[]){
	
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

	openUDPClientSocket(&udp_client,"192.168.7.2",8888);
	sendUDPClientData(&udp_client,&lisa_message,sizeof(lisa_message));
	closeUDPClientSocket(&udp_client);
	
return 0;	
}

