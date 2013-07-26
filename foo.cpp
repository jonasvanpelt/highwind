#include<iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/time.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <time.h>
#include<stdlib.h>
#include<string.h>

#define VERBOSE 1

//=========================================================================================================================
//FUNCTION PROTOTYPES - some prototypes for serial comms
//=========================================================================================================================

struct timeval tv;
long timeusec;
long som=0;
long pass=0;
long speed;
int setup(void);
int writeport(int fd, char *chars);
int readport();
int getbaud();
int initport();
void clear_sBuffer(void);
int create_port(void);

//=========================================================================================================================
//GLOBALS - GENERAL 
//=========================================================================================================================
int fd;
char sBuffer[14];
char sResult[14];

int sBufferSize = sizeof(sBuffer);

//=========================================================================================================================
//    MAIN LOOP
//=========================================================================================================================


main(int argc, char *argv[])
{
	int sBufferLength = 0;
	int i;
	if(setup() != 1)
	{
		printf("An error occured during setup");
		return 1;
	}

	fcntl(fd, F_SETFL, O_NONBLOCK); // don't block serial read
		
	while(1)
	{
		//Find number of bytes in buffer and read when enough

		int bytes;
		ioctl(fd, FIONREAD, &bytes);        //set bytes to number of bytes in buffer
		if(bytes > 25) //tweak this number to gain performance - lower equals faster read but more likely to have erroneous strings
		{
			sBuffer[0] = 0x00;

			usleep((useconds_t) 100);
#if VERBOSE

			printf("bytes: %d ", bytes);
#endif
			sBufferLength = readport();

			if (sBufferLength == -1) 
			{
				printf("read failed - sBuffer: %s\n", sBuffer);
				close(fd);
				return 1;
			}
			printf("buffer: ");
			for(i=0;i<sBufferLength;i++)
			{
				printf("%X ", sBuffer[i]);
			}
			printf("\n");
			clear_sBuffer();
		}
	}

	close(fd);
}

//=========================================================================================================================
//    SETUP
//=========================================================================================================================


int setup()
{
	if(!create_port())
	{
		return -1;
	}
	fd = open("/dev/ttyO4", O_RDWR | O_NOCTTY | O_NDELAY);     
	if (fd = -1) {
		perror("open_port: Unable to open /dev/ttyO4 - ");
		return -1;
	} else {
		fcntl(fd, F_SETFL, 0);
		printf("Serial connection established on /dev/ttyO4\n");
	}
	initport();
	printf("baud: %d\n", getbaud());

	return 1;
}

int initport()
{
	struct termios options;		//Get the current options for the port...
	tcgetattr(fd, &options);	// Set the baud rates to whatever is needed... (Max 115200)
	cfsetispeed(&options, B57600);
	cfsetos.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;

	// Set the new options for the port...

	tcsetattr(fd, TCSANOW, &options);
	return 1;
}

int getbaud() 
{
	struct termios termAttr;
	int inputSpeed = -1;
	speed_t baudRate;
	tcgetattr(fd, &termAttr);
	/* Get the input speed.                              */
	baudRate = cfgetispeed(&termAttr);
	switch (baudRate) 
	{
		case B0:      inputSpeed = 0; break;
		case B50:     inputSpeed = 50; break;
		case B110:    inputSpeed = 110; break;
		case B134:    inputSpeed = 134; break;
		case B150:    inputSpeed = 150; break;
		case B200:    inputSpeed = 200; break;
		case B300:    inputSpeed = 300; break;
		case B600:    inputSpeed = 600; break;
		case B1200:   inputSpeed = 1200; break;
		case B1800:   inputSpeed = 1800; break;
		case B2400:   inputSpeed = 2400; break;
		case B4800:   inputSpeed = 4800; break;
		case B9600:   inputSpeed = 9600; break;
		case B19200:  inputSpeed = 19200; break;
		case B38400:  inputSpeed = 38400; break;
		case B57600:  inputSpeed = 57600; break;
		case B115200: inputSpeed = 115200; break;
	}
	return inputSpeed;
}

int create_port()
{
	char  tmp[256]={0x0};
	char flag = 0;
	FILE *fp ;

	printf("Opening /sys/devices/bone_capemgr.9/slots\n");

	fp = fopen("/sys/devices/bone_capemgr.9/slots", "r");

	printf("openened \n");

	if (fp == NULL)
	{
		printf("Unable to open the file\n");
		return 0;
	}

	printf("Searching the file to check if uart5 is enabled:\n");

	while(fp!=NULL && fgets(tmp, sizeof(tmp), fp)!=NULL)
	{
		if (strstr(tmp, "enable-uart5"))
		{
			printf("Uart5 is enabled\n");
			flag = 1;
			break;
		}
	}

	fclose(fp);
	fd = open("/sys/devices/bone_capemgr.9/slots", O_RDWR);

	if (flag)
	{
		close(fd);
		return 1;
	} else {

		printf("Uart5 not enabled, trying to enable...\n");

		if (write(fd,"enable-uart5", 12)<0)
		{
			printf("Unable to enable Uart5 on device\n");
			close(fd);
			return 0;
		} else {
			printf("Enabled Uart5 on device\n");
			close(fd);
			return 1;
		}
	}
}




//=========================================================================================================================
//    READ - WRITE
//=========================================================================================================================

int readport() 
{
	int n = read(fd, sBuffer, sBufferSize);
	sBuffer[n] = 0x00;                //This is needed or the previous contents of the string will appear after the changed characters. 
	if (n < 1) 
	{
		return -1;
	}                    
	return n;
}

void clear_sBuffer()
{
	for(int i=0;i<sBufferSize;i++)
	{
		sBuffer[i]=0x00;
	}
}speed(&options, B57600);	// Enable the receiver and set local mode...

	options.c_cflag |= (CLOCAL | CREAD);
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;

	// Set the new options for the port...

	tcsetattr(fd, TCSANOW, &options);
	return 1;
}

int getbaud() 
{
	struct termios termAttr;
	int inputSpeed = -1;
	speed_t baudRate;
	tcgetattr(fd, &termAttr);
	/* Get the input speed.                              */
	baudRate = cfgetispeed(&termAttr);
	switch (baudRate) 
	{
		case B0:      inputSpeed = 0; break;
		case B50:     inputSpeed = 50; break;
		case B110:    inputSpeed = 110; break;
		case B134:    inputSpeed = 134; break;
		case B150:    inputSpeed = 150; break;
		case B200:    inputSpeed = 200; break;
		case B300:    inputSpeed = 300; break;
		case B600:    inputSpeed = 600; break;
		case B1200:   inputSpeed = 1200; break;
		case B1800:   inputSpeed = 1800; break;
		case B2400:   inputSpeed = 2400; break;
		case B4800:   inputSpeed = 4800; break;
		case B9600:   inputSpeed = 9600; break;
		case B19200:  inputSpeed = 19200; break;
		case B38400:  inputSpeed = 38400; break;
		case B57600:  inputSpeed = 57600; break;
		case B115200: inputSpeed = 115200; break;
	}
	return inputSpeed;
}

int create_port()
{
	char  tmp[256]={0x0};
	char flag = 0;
	FILE *fp ;

	printf("Opening /sys/devices/bone_capemgr.9/slots\n");

	fp = fopen("/sys/devices/bone_capemgr.9/slots", "r");

	printf("openened \n");

	if (fp == NULL)
	{
		printf("Unable to open the file\n");
		return 0;
	}

	printf("Searching the file to check if uart5 is enabled:\n");

	while(fp!=NULL && fgets(tmp, sizeof(tmp), fp)!=NULL)
	{
		if (strstr(tmp, "enable-uart5"))
		{
			printf("Uart5 is enabled\n");
			flag = 1;
			break;
		}
	}

	fclose(fp);
	fd = open("/sys/devices/bone_capemgr.9/slots", O_RDWR);

	if (flag)
	{
		close(fd);
		return 1;
	} else {

		printf("Uart5 not enabled, trying to enable...\n");

		if (write(fd,"enable-uart5", 12)<0)
		{
			printf("Unable to enable Uart5 on device\n");
			close(fd);
			return 0;
		} else {
			printf("Enabled Uart5 on device\n");
			close(fd);
			return 1;
		}
	}
}




//=========================================================================================================================
//    READ - WRITE
//=========================================================================================================================

int readport() 
{
	int n = read(fd, sBuffer, sBufferSize);
	sBuffer[n] = 0x00;                //This is needed or the previous contents of the string will appear after the changed characters. 
	if (n < 1) 
	{
		return -1;
	}                    
	return n;
}

void clear_sBuffer()
{
	for(int i=0;i<sBufferSize;i++)
	{
		sBuffer[i]=0x00;
	}
}
