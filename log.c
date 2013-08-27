/*
 * AUTHOR: Jonas Van Pelt
 */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include "header_files/log.h"


#ifndef DEBUG 
#define DEBUG 0
#endif

/********************************
 * PROTOTYPES PRIVATE
 * ******************************/
 
static LOG_errCode mount_sd_card();
 
/********************************
 * GLOBALS
 * ******************************/
 
FILE *lisa_log_file,*groundstation_log_file,*boneplane_log_file;


/********************************
 * FUNCTIONS
 * ******************************/
 
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

static LOG_errCode mount_sd_card(){
	#if DEBUG  > 1
		printf("Entering mount_sd_card\n");
	#endif
	
	char str[256];
	strcpy (str,"mount ");
	strcat (str,SD_CARD_DEVICE_LOCATION);
	strcat (str," ");
	strcat (str,SD_CARD_MOUNT_LOCATION);
	if(system(str)!=0){
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

LOG_errCode write_data_lisa_log(char *data,int length){
	#if DEBUG  > 1
		printf("Entering write_data_lisa_log\n");
	#endif
	
	if(fwrite(data,length, 1, lisa_log_file)<1){
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

LOG_errCode write_data_groundstation_log(char *data,int length){
	#if DEBUG  > 1
		printf("Entering write_data_groundstation_log\n");
	#endif
	
	if(fwrite(data,length, 1, groundstation_log_file)<0){
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

