#include <stdlib.h>
#include <stdio.h>
#include "analyze.h"

/********************************
 * PROTOTYPES PRIVATE
 * ******************************/
 int timeval_subtract(timeval *result,timeval *t2,timeval *t1);
 

/********************************
 * FUNCTIONS
 * ******************************/
 void init_analyze(Analyze *an,int buffsize){
	an->buffsize=buffsize;
	an->count=0;
	an->sum=0;
	an->avg=0;
	an->is_first_data=1;
	an->buffer=(double *)malloc(sizeof(double)*buffsize);
	if(an->buffer == NULL){
			printf("could not allocate memory for buffer\n");
	}
}
 
int calculate_period(Analyze *an,timeval tvSent){
	if(an->index < an->buffsize){
		
		if(an->is_first_data){
			an->is_first_data=0;
		}else{
			timeval tvResult;
			double diff;
			timeval_subtract(&tvResult,&tvSent,&an->previous_timestamp);
			diff = ((double)tvResult.tv_sec * 1e6 + tvResult.tv_usec) * 1e-3;
			an->buffer[an->index]=diff;
			an->sum+=diff;
			an->index++;
		}
		an->previous_timestamp=tvSent;
		return 0;
	}else{
		an->avg=an->sum/an->count;
		return 1;
	}
	
}

void print_avg(Analyze *an){
	printf("avg %f\n",an->avg);
}

void dump_buffer_to_file(Analyze *an,const char *file_name){
	FILE *file; 
	int i;
	file = fopen(file_name,"w"); 
	for(i=0;i<an->buffsize;i++){
		fprintf(file,"%f\n", an->buffer[i]);
	}
	fclose(file);
}

int timeval_subtract(timeval *result,timeval *t2,timeval *t1)
{
    long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;
    return (diff<0);	//Return 1 if the difference is negative, otherwise 0. 
}

void destroy_analyze(Analyze *an){
	an->buffsize=0;
	an->count=0;
	an->sum=0;
	an->avg=0;
	free(an->buffer);
}
