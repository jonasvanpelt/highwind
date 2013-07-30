#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "log.h"
#include "circular_buffer.h"

#define CBSIZE 64

/************************************
 * PROTOTYPES
 * **********************************/
 void *leesbuf(void *arg);


 /***********************************
  * GLOBALS
  * *********************************/


//log buffer for data from lisa
 CircularBuffer cb;

 /***********************************
  * MAIN
  * *********************************/
int main(int argc, char *argv[]){
	
	//init cirucal data log buffers
	 cbInit(&cb, CBSIZE);
	 
	//this variable is our reference to the second thread
	pthread_t thread;
	

	//create a second thread which executes lisa_to_pc
	if(pthread_create(&thread, NULL,leesbuf,NULL)) {
		printf("thread exit");
		exit(1);
	}	
	
	/*-------------------------START OF FIRST THREAD: PC TO LISA------------------------*/
	ElemType cb_elem = {0};
	
	int i;
	
	while(1){
		while(!cbIsFull(&cb)){
			cb_elem.value[0]=i;
			cbWrite(&cb, &cb_elem);
			i++;
			usleep(10);
		}
			printf("buffer full\n");
	}
	

	/*------------------------END OF FIRST THREAD------------------------*/
	
	
	//wait for the second thread to finish
	if(pthread_join(thread, NULL)) {
		exit(EXIT_FAILURE);
	}
	
	cbFree(&cb);
	
	return 0;
}
void *leesbuf(void *arg){
/*-------------------------START OF SECOND THREAD: LISA TO PC------------------------*/	


	ElemType cb_elem = {0};

	//read data from UART

	open_data_lisa_log();

	while(1){

		while (!cbIsEmpty(&cb)) {
			cbRead(&cb, &cb_elem);
			write_data_lisa_log(cb_elem.value);
		}

	}
	close_data_lisa_log();



	return NULL;
/*------------------------END OF SECOND THREAD------------------------*/

}	





