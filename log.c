#include <stdio.h>
#include <time.h>
#include "log.h"

void error_write(char *file_name,char *function,char *message){
	//write error to log		
	FILE *file; 
	time_t now = time(0);
	char* time_string;
	time_string = ctime(&now);
    file = fopen("error.txt","a+"); 
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
    file = fopen("log.txt","a+"); 
	fprintf(file,"%s%s-%s\t%s\n\n",time_string,file_name,function,message); 
	fclose(file); 	
}
