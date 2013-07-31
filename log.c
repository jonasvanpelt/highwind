/*
 * AUTHOR: Jonas Van Pelt
 */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include "log.h"

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
	//check if sdcard is present
	FILE *file;
	file = fopen("/media/sdcard/sdcard_present.txt","r"); 
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
	char str[80];
	strcpy (str,"mount ");
	strcpy (str,SD_CARD_DEVICE_LOCATION);
	strcpy (str," ");
	strcpy (str,SD_CARD_MOUNT_LOCATION);

	return system(str);
}

/**********************************
LOG FOR DATA COMING FROM LISA
***********************************/
int open_data_lisa_log(){
	//open data log file
	lisa_log_file = fopen(FILE_PATH_LISA_LOG,"a+"); 
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
	groundstation_log_file = fopen(FILE_PATH_GROUND_LOG,"a+"); 
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
	boneplane_log_file = fopen(FILE_PATH_BONEPLANE_LOG,"a+"); 
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
    file = fopen(FILE_PATH_PROGRAM_ERROR,"a+"); 
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
    file = fopen(FILE_PATH_PROGRAM_LOG,"a+"); 
	fprintf(file,"%s%s-%s\t%s\n\n",time_string,file_name,function,message); 
	fclose(file); 	
}

//NOG BOODSCHAPPEN KUNNNEN DOORSTUREN NAAR SERVER OOK, ERRORS VB
