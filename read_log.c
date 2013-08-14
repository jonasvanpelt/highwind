#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "udp_communication.h"

#define BUFF_SIZE 255

int main(int argc, char **argv)
{
    char buff[BUFF_SIZE];
    //FILE *f = fopen("/media/sdcard/data_lisa_log.txt", "r");
    FILE *f = fopen("/media/sdcard/data_groundstation_log.txt", "r");
    int ch;
    int length;
    int i;
    static UDP udp_client;

ch = fgetc(f);	

openUDPClientSocket(&udp_client,"10.33.136.11",8888,10000000);


while(ch!=EOF){

	if(ch!=0x99){
		printf("wrong startbyte\n");
		exit(1);
	}else{
		buff[0]=ch;
		length=fgetc(f);
		buff[1]=length;
		for(i=2;i<length;i++){
			buff[i]=fgetc(f);
		}

		for(i=0;i<length;i++){
			printf("%d ",buff[i]);
		}
		printf("\n\n");

		//sendUDPClientData(&udp_client,&buff,length);

	}
	usleep(1000);
	ch = fgetc(f);
}
closeUDPClientSocket(&udp_client);

    fclose(f);
    return 0;
}
