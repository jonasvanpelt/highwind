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
	int count;
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
int calculate_period(Analyze *an,timeval tvSent); /*calculates the period between current and previous timestamp and stores it in buffer in milliseconds*/
void dump_buffer_to_file(Analyze *an,const char *file_name);
void destroy_analyze(Analyze *an);

#endif /*ANALYZE_H_*/

