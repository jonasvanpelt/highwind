#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "udp_communication.h"
#include "data_decoding.h"
#include "log.h"

#define MAX_INPUT_STREAM_SIZE 255
#define MAX_OUTPUT_STREAM_SIZE 34

#ifndef DEBUG 
#define DEBUG 0
#endif


/************************************
 * PROTOTYPES
 * **********************************/

void *server_to_planebone(void *connection);

 /***********************************
  * GLOBALS
  * *********************************/
  
static char FILENAME[] = "main_full_server.c";

    
typedef struct{
		int port_number_server_to_planebone;
		int port_number_planebone_to_server;
		char *planebone_ip;
} Connection;


 /***********************************
  * MAIN
  * *********************************/
int main(int argc, char *argv[]){
	Connection connection;

	//parse arguments	
	if(argc == 4){
		//first argument is always name of program or empty string
		connection.planebone_ip=argv[1];
		connection.port_number_server_to_planebone=atoi(argv[2]);	
		connection.port_number_planebone_to_server=atoi(argv[3]);
	}else{
			printf("wrong parameters: planebone ip - send port number - receive port number\n");
			exit(EXIT_FAILURE);		
	}
	
	pthread_t thread_server_to_planebone;

	//create a second thread which executes server_to_planebone
	if(pthread_create(&thread_server_to_planebone, NULL, server_to_planebone,&connection)) {
		error_write(FILENAME,"main()","error creating lisa thread");
		exit(EXIT_FAILURE);
	}	
	
	/*-------------------------START OF FIRST THREAD: PLANEBONE TO SERVER------------------------*/
	
	static UDP udp_server;
	uint8_t input_stream[MAX_INPUT_STREAM_SIZE];

	openUDPServerSocket(&udp_server,connection.port_number_planebone_to_server);
	
	//init the data decode pointers
	init_decoding();


	while(1){
		//printf("\nWaiting for data...\n\n");
		fflush(stdout);

		//1. retreive UDP data form planebone from ethernet port.
		
		receiveUDPServerData(&udp_server,(void *)&input_stream,sizeof(input_stream)); //blocking !!!
		
		#if DEBUG > 0
		
		printf("data raw: ");
		int i;
		for(i=0;i<input_stream[1];i++){
				printf("%d ",input_stream[i]);
		}
		printf("\n");
		
		printf("start hex: %x\n", input_stream[0]);
		printf("length: %d\n", input_stream[1]);
		printf("send id: %d\n", input_stream[2]);
		printf("message id: %d\n", input_stream[3]);
		printf("checksum1: %d\n", input_stream[input_stream[1]-2]);
		printf("checksum2: %d\n", input_stream[input_stream[1]-1]);
		printf("\n");
		
		#endif
		
		//2. decode data 
		Data* data = get_read_pointer();
		
		if(data_update(input_stream)==-1){ 
				char str[50];
				sprintf(str, "error decoding data package with length = %d and sender_id = %d and message_id = %d",input_stream[1],input_stream[2], input_stream[3]);
				printf("\n\nERROR DECODING !!!\n\n");
				error_write(FILENAME,"main()",str);	
				//exit(1);
			}else{
				switch_read_write(); //only switch read write if data decoding was succesfull
				printf("data decoding succeeded\n");
				
				/*printf("gyro_raw content\n");
				printf("gp %d\n",read->lisa_plane.imu_gyro_raw.message.gp);
				printf("gq %d\n",read->lisa_plane.imu_gyro_raw.message.gq);
				printf("gr %d\n",read->lisa_plane.imu_gyro_raw.message.gr);*/
				
				if(input_stream[3]==221){

					
					int i;
					printf("Baro_raw content:");

					for(i=0;i<input_stream[1];i++){
						printf("%d ",data->lisa_plane.baro_raw.raw[i]);
					}
					printf("\n");
					printf("abs %d\n",data->lisa_plane.baro_raw.message.abs);
					printf("diff %d\n",data->lisa_plane.baro_raw.message.diff);
				}
			
				printf("\n");
				if(input_stream[3]==203){
					int i;
					printf("Imu_gyro_raw content:");

					for(i=0;i<input_stream[1];i++){
						printf("%d ",data->lisa_plane.imu_gyro_raw.raw[i]);
					}
					printf("\n");
					printf("gp %d\n",data->lisa_plane.imu_gyro_raw.message.gp);
					printf("gp %d\n",data->lisa_plane.imu_gyro_raw.message.gp);
					printf("gr %d\n",data->lisa_plane.imu_gyro_raw.message.gr);

				}
				
			}
		
	}

	closeUDPServerSocket(&udp_server);

	/*------------------------END OF FIRST THREAD------------------------*/

	//wait for the second thread to finish
	if(pthread_join(thread_server_to_planebone, NULL)) {
		error_write(FILENAME,"main()","error joining thread_lisa_to_pc");
		exit(EXIT_FAILURE);
	}

	return 0;
}

void *server_to_planebone(void *connection){
/*-------------------------START OF SECOND THREAD: SERVER TO PLANEBONE------------------------*/		
	
	Connection *conn=(Connection *)connection;
	static UDP udp_client;
	
	openUDPClientSocket(&udp_client,conn->planebone_ip,conn->port_number_server_to_planebone);
	int i=0;
	
	while(1)
	{
		//1. read data from i do'nt now where
		
		
		uint8_t encoded_data[MAX_OUTPUT_STREAM_SIZE];
		Output output;

		//create test data

		output.servo_commands[0]=i;
		output.servo_commands[1]=0;
		output.servo_commands[2]=0;
		output.servo_commands[3]=0;
		output.servo_commands[4]=0;
		output.servo_commands[5]=0;
		output.servo_commands[6]=0;
		
		if(i==1000){
				i=5;
		}else{
			i=1000;
		}
	
		//2. encode the data		
		
		data_encode(output.raw,encoded_data,1,52);

		//3. send data to eth port using UDP
		sendUDPClientData(&udp_client,&encoded_data,sizeof(encoded_data));	
		
		//sleep(1);
		usleep(100000);
	}
	closeUDPClientSocket(&udp_client);



	return NULL;
/*------------------------END OF SECOND THREAD------------------------*/

}	




