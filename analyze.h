/*
 * AUTHOR: Jonas Van Pelt
 */
#ifndef ANALYZE_H_ 
#define ANALYZE_H_

#include <sys/time.h>
#include <time.h>


/********************************
 * GLOBALS
 * ******************************/
typedef struct timeval timeval;

typedef struct{
	double sum;
	int buffsize;
	int index;
	double avg;
	int is_first_data;
	timeval previous_timestamp;
	double *buffer;
}Analyze;



/********************************
 * PROTOTYPES PUBLIC
 * ******************************/
 
void init_analyze(Analyze *an,int buffsize);
int calculate_frequency(Analyze *an,timeval tvSent); /*calculates the freq of the package in hz*/
int calculate_latency(Analyze *an,timeval tvSent,timeval tvNow); /*calculates the latency between tvNow and tvSent*/
void dump_buffer_to_file(Analyze *an,const char *file_name);
void destroy_analyze(Analyze *an);
double get_avg(Analyze *an);
#endif /*ANALYZE_H_*/

