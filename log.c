#include <stdio.h>
#include <time.h>
#include "log.h"

FILE *lisa_log_file,*groundstation_log_file,*boneplane_log_file;

int init_log(){
	//check if sdcard is present
	FILE *file;
	file = fopen("/media/sdcard/sdcard_present.txt","r"); 
	if(file==NULL){
			//SD CARD IS NOT PRESENT!
			//send error message to server
		return -1;
	}
	return 0;
}

/**********************************
LOG FOR DATA COMING FROM LISA
***********************************/
int open_data_lisa_log(){
	//open data log file
	lisa_log_file = fopen("/media/sdcard/data_lisa_log.txt","a+"); 
	if(lisa_log_file==NULL){
			return -1;
	}
	return 0;
}

int write_data_lisa_log(char *data){
	return fprintf(lisa_log_file,"%s\n",data); 
}

int close_data_lisa_log(){
	return fclose(lisa_log_file);
}

/**********************************
LOG FOR DATA COMING FROM GROUNDSTATION
***********************************/
int open_data_groundstation_log(){
	//open data log file
	groundstation_log_file = fopen("/media/sdcard/data_groundstation_log.txt","a+"); 
	if(groundstation_log_file==NULL){
			return -1;
	}
	return 0;
}

int write_data_groundstation_log(char *data){
	return fprintf(groundstation_log_file,"%s\n",data); 
}

int close_data_groundstation_log(){
	return fclose(groundstation_log_file);
}


/**********************************
LOG FOR DATA COMING FROM BONEPLANE
***********************************/
int open_data_boneplane_log(){
	//open data log file
	boneplane_log_file = fopen("/media/sdcard/data_boneplane_log.txt","a+"); 
	if(boneplane_log_file==NULL){
			return -1;
	}
	return 0;
}

int write_data_boneplane_log(char *data){
	return fprintf(boneplane_log_file,"%s\n",data); 
}

int close_data_boneplane_log(){
	return fclose(boneplane_log_file);
}


/**********************************
LOG FOR ERRORS FROM PROGRAM 
***********************************/
void error_write(char *file_name,char *function,char *message){
	//write error to log		
	FILE *file; 
	time_t now = time(0);
	char* time_string;
	time_string = ctime(&now);
    file = fopen("/media/sdcard/error.txt","a+"); 
	fprintf(file,"%s%s-%s\t%s\n\n",time_string,file_name,function,message); 
	fclose(file); 	
}

void log_write(char *file_name,char *function,char *message)
{
	//write error to log		
	FILE *file; 
	time_t now = time(0);
	char* time_string;
	time_string = ctime(&now);
    file = fopen("/media/sdcard/log.txt","a+"); 
	fprintf(file,"%s%s-%s\t%s\n\n",time_string,file_name,function,message); 
	fclose(file); 	
}

//NOG BOODSCHAPPEN KUNNNEN DOORSTUREN NAAR SERVER OOK, ERRORS VB
