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

LOG_errCode init_log(){
	#if DEBUG  > 1
		printf("Entering init_log\n");
	#endif
	
	//check if sdcard is present
	FILE *file;
	file = fopen("/media/sdcard/sdcard_present","r"); 
	if(file==NULL){	
		//SD CARD IS NOT PRESENT!, try mounting it	
		return mount_sd_card();
	}
	return LOG_ERR_NONE;
}

LOG_errCode mount_sd_card(){
	#if DEBUG  > 1
		printf("Entering mount_sd_card\n");
	#endif
	
	char str[256];
	strcpy (str,"mount ");
	strcat (str,SD_CARD_DEVICE_LOCATION);
	strcat (str," ");
	strcat (str,SD_CARD_MOUNT_LOCATION);
	if(system(str)==-1){
		return LOG_ERR_MOUNT_SD;
	}

	return LOG_ERR_NONE;
}

/**********************************
LOG FOR DATA COMING FROM LISA
***********************************/
LOG_errCode open_data_lisa_log(){
	#if DEBUG  > 1
		printf("Entering open_data_lisa_log\n");
	#endif
	
	lisa_log_file = fopen(FILE_PATH_LISA_LOG,"a+"); 
	if(lisa_log_file==NULL){
			return LOG_ERR_OPEN_FILE;
	}
	return LOG_ERR_NONE;
}

LOG_errCode write_data_lisa_log(char *data){
	#if DEBUG  > 1
		printf("Entering write_data_lisa_log\n");
	#endif
	
	if(fprintf(lisa_log_file,"%s\n",data)<0){
		return LOG_ERR_WRITE; 
	}
	return LOG_ERR_NONE;
}

LOG_errCode close_data_lisa_log(){
	#if DEBUG  > 1
		printf("Entering close_data_lisa_log\n");
	#endif
	
	if(fclose(lisa_log_file)==EOF){
		return LOG_ERR_CLOSE; 
	}
	return LOG_ERR_NONE;
}

/**********************************
LOG FOR DATA COMING FROM GROUNDSTATION
***********************************/
LOG_errCode open_data_groundstation_log(){
	#if DEBUG  > 1
		printf("Entering open_data_groundstation_log\n");
	#endif
	
	groundstation_log_file = fopen(FILE_PATH_GROUND_LOG,"a+"); 
	if(groundstation_log_file==NULL){
			return LOG_ERR_OPEN_FILE;
	}
	return LOG_ERR_NONE;
}

LOG_errCode write_data_groundstation_log(char *data){
	#if DEBUG  > 1
		printf("Entering write_data_groundstation_log\n");
	#endif
	
	if(fprintf(groundstation_log_file,"%s\n",data)<0){
		return LOG_ERR_WRITE; 
	}
	return LOG_ERR_NONE;
}

LOG_errCode close_data_groundstation_log(){
	#if DEBUG  > 1
		printf("Entering close_data_groundstation_log\n");
	#endif
	
	if(fclose(groundstation_log_file)==EOF){
		return LOG_ERR_CLOSE; 
	}
	return LOG_ERR_NONE;	
}


/**********************************
LOG FOR ERRORS FROM PROGRAM 
***********************************/
LOG_errCode error_write(char *file_name,char *message){
	#if DEBUG  > 1
		printf("Entering error_write\n");
	#endif
	
	FILE *file; 
	time_t now = time(0);
	char* time_string;
	time_string = ctime(&now);
    file = fopen(FILE_PATH_PROGRAM_ERROR,"a+"); 
    
    if(file==NULL){
		return LOG_ERR_OPEN_FILE;
	}
     
	if(fprintf(file,"%s%s\t%s\n\n",time_string,file_name,message)<0){
		return LOG_ERR_WRITE; 
	}
	
	if(fclose(file)==EOF){
		return LOG_ERR_CLOSE; 
	}
	
	return LOG_ERR_NONE;
}

LOG_errCode log_write(char *file_name,char *message){
	#if DEBUG  > 1
		printf("Entering log_write\n");
	#endif
	
	//write error to log		
	FILE *file; 
	time_t now = time(0);
	char* time_string;
	time_string = ctime(&now);
    file = fopen(FILE_PATH_PROGRAM_LOG,"a+"); 
    
     if(file==NULL){
		return LOG_ERR_OPEN_FILE;
	}
    
	if(fprintf(file,"%s%s\t%s\n\n",time_string,file_name,message)<0){
		return LOG_ERR_WRITE; 
	}
	
	if(fclose(file)==EOF){
		return LOG_ERR_CLOSE; 
	}	
}

