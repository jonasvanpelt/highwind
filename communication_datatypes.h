#ifndef COMMUNCATION_DATATYPES_H_ 
#define COMMUNCATION_DATATYPES_H_

#include<stdint.h>

typedef struct{
	int32_t abs;
	int32_t diff;
} Barometer;


typedef struct{
	uint8_t start;
	uint8_t length; 
	uint8_t aircraft_id;
	uint8_t checksum_A;
	uint8_t checksum_B;
	Barometer barometer;
} Lisa_message; 

#endif /*COMMUNCATION_DATATYPES_H_*/

