/*
 * AUTHOR: Jonas Van Pelt
 */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include "log.h"


#ifndef DEBUG 
#define DEBUG 0
#endif

FILE *lisa_log_file,*groundstation_log_file,*boneplane_log_file;

static const char FILE_PATH_LISA_LOG[] = "/media/sdcard/data_lisa_log.txt";
static const char FILE_PATH_GROUND_LOG[] = "/media/sdcard/data_groundstation_log.txt";
static const char FILE_PATH_BONEPLANE_LOG[] = "/media/sdcard/data_boneplane_log.txt";

/*static const char FILE_PATH_PROGRAM_LOG[]="/media/sdcard/log.txt";
static const char FILE_PATH_PROGRAM_ERROR[]="/media/sdcard/error.txt";*/

static const char FILE_PATH_PROGRAM_LOG[]="log.txt";
static const char FILE_PATH_PROGRAM_ERROR[]="error.txt";

static const char SD_CARD_MOUNT_LOCATION[] = "/media/sdcard/";
static const char SD_CARD_DEVICE_LOCATION[] = "/dev/mmcblk0p2";

/*static const char FILE_PATH_LISA_LOG[] = "data_lisa_log.txt";
static const char FILE_PATH_GROUND_LOG[] = "data_groundstation_log.txt";
static const char FILE_PATH_BONEPLANE_LOG[] = "data_boneplane_log.txt";*/

int init_log(){
	#if DEBUG  > 1
		printf("Entering init_log\n");
	#endif
	
	//check if sdcard is present
	FILE *file;
	file = fopen("/media/sdcard/sdcard_present","r"); 
	if(file==NULL){	
			//SD CARD IS NOT PRESENT!, try mounting it
			if(mount_sd_card()==-1)
			{
				//sd card cannot be mount
				return -1;
			}	
	}
	return 0;
}

int mount_sd_card(){
	#if DEBUG  > 1
		printf("Entering mount_sd_card\n");
	#endif
	
	char str[256];
	strcpy (str,"mount ");
	strcat (str,SD_CARD_DEVICE_LOCATION);
	strcat (str," ");
	strcat (str,SD_CARD_MOUNT_LOCATION);

	return system(str);
}

/**********************************
LOG FOR DATA COMING FROM LISA
***********************************/
int open_data_lisa_log(){
	#if DEBUG  > 1
		printf("Entering open_data_lisa_log\n");
	#endif
	
	//open data log file
	lisa_log_file = fopen(FILE_PATH_LISA_LOG,"a+"); 
	if(lisa_log_file==NULL){
			return -1;
	}
	return 0;
}

int write_data_lisa_log(char *data){
	#if DEBUG  > 1
		printf("Entering write_data_lisa_log\n");
	#endif
	
	return fprintf(lisa_log_file,"%s\n",data); 
}

int close_data_lisa_log(){
	#if DEBUG  > 1
		printf("Entering close_data_lisa_log\n");
	#endif
	
	return fclose(lisa_log_file);
}

/**********************************
LOG FOR DATA COMING FROM GROUNDSTATION
***********************************/
int open_data_groundstation_log(){
	#if DEBUG  > 1
		printf("Entering open_data_groundstation_log\n");
	#endif
	
	//open data log file
	groundstation_log_file = fopen(FILE_PATH_GROUND_LOG,"a+"); 
	if(groundstation_log_file==NULL){
			return -1;
	}
	return 0;
}

int write_data_groundstation_log(char *data){
	#if DEBUG  > 1
		printf("Entering write_data_groundstation_log\n");
	#endif
	
	return fprintf(groundstation_log_file,"%s\n",data); 
}

int close_data_groundstation_log(){
	#if DEBUG  > 1
		printf("Entering close_data_groundstation_log\n");
	#endif
	
	return fclose(groundstation_log_file);
}


/**********************************
LOG FOR ERRORS FROM PROGRAM 
***********************************/
void error_write(char *file_name,char *message){
	#if DEBUG  > 1
		printf("Entering error_write\n");
	#endif
	
	//write error to log		
	FILE *file; 
	time_t now = time(0);
	char* time_string;
	time_string = ctime(&now);
    file = fopen(FILE_PATH_PROGRAM_ERROR,"a+"); 
	fprintf(file,"%s%s\t%s\n\n",time_string,file_name,message); 
	fclose(file); 	
}

void log_write(char *file_name,char *message){
	#if DEBUG  > 1
		printf("Entering log_write\n");
	#endif
	
	//write error to log		
	FILE *file; 
	time_t now = time(0);
	char* time_string;
	time_string = ctime(&now);
    file = fopen(FILE_PATH_PROGRAM_LOG,"a+"); 
	fprintf(file,"%s%s\t%s\n\n",time_string,file_name,message); 
	fclose(file); 	
}

