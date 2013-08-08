/*
 * AUTHOR: Maarten Arits and Jonas Van Pelt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <poll.h>

#include "uart_communication.h"

#ifndef DEBUG 
#define DEBUG 0
#endif


/********************************
 * PROTOTYPES PRIVATE
 * ******************************/
 
extern UART_errCode serial_port_setup(void); //returns the number of read bytes
extern int serial_port_read(uint32_t length);
extern UART_errCode serial_port_create(void);
extern int serial_port_get_baud(void);
extern UART_errCode  serial_port_open_raw(const char* device, speed_t speed);
extern UART_errCode  serial_port_open(const char* device, void(*term_conf_callback)(struct termios*, speed_t*));
extern void serial_port_free(void);
extern void serial_port_flush(void);
extern UART_errCode serial_port_flush_input(void);
extern UART_errCode serial_port_flush_output(void);
extern void serial_buffer_clear(void);
extern void serial_output_buffer_clear(void);
extern void serial_input_buffer_clear(void);
extern void benchmark_start(int timer);
extern void benchmark_stop(int timer);
extern void packets_clear(void);
extern uint8_t serial_port_get_length(void);

/********************************
 * GLOBALS
 * ******************************/
 
static const char FILENAME[] = "uart_communication.c";

//config
speed_t speed = B57600;
const char device[]="/dev/ttyO4";
int serial_input_buffer_size = sizeof(serial_input.buffer);

/********************************
 * FUNCTIONS
 * ******************************/

int wait_for_data(){
	//todo: some error handling here
	struct pollfd fds[1];
	int timeout = (3 * 1 * 1000); //time out of 3 seconds
	fds[0].fd=serial_stream->fd;
	fds[0].events=POLLIN;
	poll(fds,1,timeout); //block until there is data in the serial stream
}
 
int serial_input_check() //returns the number of read bytes
{
	#if DEBUG  > 1
		printf("Entering serial_input_check\n");
	#endif
	
	int i;
	int serial_input_buffer_chars;
	uint8_t checksum_1;
	uint8_t checksum_2;
	uint8_t message_length;
	struct timeval start;
	struct timeval current;
	int flag_time=0;

	benchmark_start(0);
	//Find number of bytes in buffer and read when enough
	  
	wait_for_data();   
	ioctl(serial_stream->fd, FIONREAD, &serial_input_buffer_chars);        //set to number of bytes in buffer
	if(serial_input_buffer_chars > 0) //look for char in buffer
	{
		
		message_length = serial_port_get_length();

		if(message_length == 0){
			packets.serial.lost++;
			return UART_ERR_READ;
		}

		gettimeofday(&start, NULL);
		
		serial_input_buffer_chars=0;
		while(serial_input_buffer_chars<message_length-2){
			wait_for_data(); 
			ioctl(serial_stream->fd, FIONREAD, &serial_input_buffer_chars);        
			
		}

		serial_input_buffer_chars = serial_port_read(message_length-2); //reads the port out and stores the number of chars red
	
		if (serial_input_buffer_chars == -1) 
		{
			packets.serial.lost++;
			return UART_ERR_READ;

		} else {

			checksum_1 = message_length; //part of the checksum
			checksum_2 = checksum_1; // add checksum_0 to 0

			for(i=0;i<message_length-4;i++) // count until checksum 1 --> length - 2 (checksums) - 2 (ofset)
			{
				checksum_1 += (uint8_t) serial_input.buffer[i];
				checksum_2 += checksum_1;
			}


			if (serial_input.buffer[message_length-4]!= checksum_1 || serial_input.buffer[message_length-3] != checksum_2)
			{
				serial_port_flush_input();
				packets.serial.lost++;
				return UART_ERR_READ; //-1

			} else {

				//first two bits (start and length ) should be in buffer, now
				char temp[message_length-2];
				int i;
				for(i=0;i<message_length-2;i++)
				{
					temp[i]=serial_input.buffer[i];	
				}
				serial_input_buffer_clear();
				serial_input.buffer[0]=0x99;
				serial_input.buffer[1]=message_length;
				for(i=2;i<message_length;i++)
				{
					serial_input.buffer[i]=temp[i-2];	
				}
				

				packets.serial.received++;

				#if DEBUG > 0
					printf("\nraw: ");
					for(i=0;i<message_length;i++){
						printf("%d ",serial_input.buffer[i]);
					}
					printf("\n");	

					printf("start: %X ", serial_input.buffer[0]);
					printf("length: %d ", serial_input.buffer[1]);
					printf("checksum 1 calc: %d ", checksum_1);
					printf("checksum 2 calc: %d ", checksum_2);
					printf("lost / received: %d / %d ", packets.serial.lost, packets.serial.received);
					printf("\n");
				#endif
				benchmark_stop(0);
			}
		}

	}else{
		
		return UART_ERR_READ; //-1
	}
	return message_length;
}

void packets_clear(void)
{
	#if DEBUG  > 1
		printf("Entering packets_clear\n");
	#endif
	
	packets.serial.received=0;
	packets.serial.lost=0;
	packets.udp.received=0;
	packets.udp.lost=0;
}

serial_port* serial_port_new(void) {
	#if DEBUG  > 1
		printf("Entering serial_port_new\n");
	#endif
	
	serial_port* serial_stream = (serial_port*) malloc(sizeof(serial_port));
	return serial_stream;
}

void serial_port_free(void) {
	#if DEBUG  > 1
		printf("Entering serial_port_free\n");
	#endif
	
	free(serial_stream);
}

void serial_port_flush(void) {
	#if DEBUG  > 1
		printf("Entering serial_port_flush\n");
	#endif
	/*
	 * flush any input and output on the port
	 */
	serial_port_flush_input();
	serial_port_flush_output();
}


UART_errCode serial_port_flush_input(void) {
	#if DEBUG  > 1
		printf("Entering serial_port_flush_input\n");
	#endif
	/*
	 * flush any input that might be on the port so we start fresh.
	 */
	if (tcflush(serial_stream->fd, TCIFLUSH)) {
		return UART_ERR_SERIAL_PORT_FLUSH_INPUT;
	}
	return UART_ERR_NONE;
}

UART_errCode serial_port_flush_output(void) {
	#if DEBUG  > 1
		printf("Entering serial_port_flush_output\n");
	#endif
	/*
	 * flush any input that might be on the port so we start fresh.
	 */
	if (tcflush(serial_stream->fd, TCOFLUSH)) {
			return UART_ERR_SERIAL_PORT_FLUSH_OUTPUT;

	}
	return UART_ERR_NONE;
}

UART_errCode  serial_port_open_raw(const char* device, speed_t speed) {
	#if DEBUG  > 1
		printf("Entering serial_port_open_raw\n");
	#endif
	if ((serial_stream->fd = open(device, O_RDWR | O_NONBLOCK | O_NOCTTY)) < 0) {
		return UART_ERR_SERIAL_PORT_OPEN;
	}
	if (tcgetattr(serial_stream->fd, &serial_stream->orig_termios) < 0) {
		close(serial_stream->fd);
		return UART_ERR_SERIAL_PORT_OPEN;
	}
	serial_stream->cur_termios = serial_stream->orig_termios;
	/* input modes  */
	serial_stream->cur_termios.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|INPCK|ISTRIP|INLCR|IGNCR
			|ICRNL |IUCLC|IXON|IXANY|IXOFF|IMAXBEL);
	serial_stream->cur_termios.c_iflag |= IGNPAR;
	/* control modes*/
	serial_stream->cur_termios.c_cflag &= ~(CSIZE|PARENB|CRTSCTS|PARODD|HUPCL|CSTOPB);
	serial_stream->cur_termios.c_cflag |= CREAD|CS8|CLOCAL;
	/* local modes  */
	serial_stream->cur_termios.c_lflag &= ~(ISIG|ICANON|IEXTEN|ECHO|FLUSHO|PENDIN);
	serial_stream->cur_termios.c_lflag |= NOFLSH;
	if (cfsetispeed(&serial_stream->cur_termios, speed)) {
		close(serial_stream->fd);
		return UART_ERR_SERIAL_PORT_OPEN;
	}
	if (tcsetattr(serial_stream->fd, TCSADRAIN, &serial_stream->cur_termios)) {
		close(serial_stream->fd);
		return UART_ERR_SERIAL_PORT_OPEN;
	}
	serial_port_flush();
	return UART_ERR_NONE;
}

UART_errCode  serial_port_open(const char* device, void(*term_conf_callback)(struct termios*, speed_t*)) {
	#if DEBUG  > 1
		printf("Entering serial_port_open\n");
	#endif

	speed_t speed;
	if ((serial_stream->fd = open(device, O_RDWR | O_NONBLOCK)) < 0) {
		return UART_ERR_SERIAL_PORT_OPEN;
	}
	if (tcgetattr(serial_stream->fd, &serial_stream->orig_termios) < 0) {
		close(serial_stream->fd);
		return UART_ERR_SERIAL_PORT_OPEN;
	}
	serial_stream->cur_termios = serial_stream->orig_termios;
	term_conf_callback(&serial_stream->cur_termios, &speed);
	if (cfsetispeed(&serial_stream->cur_termios, speed)) {
		close(serial_stream->fd);
		return UART_ERR_SERIAL_PORT_OPEN;
	}
	if (tcsetattr(serial_stream->fd, TCSADRAIN, &serial_stream->cur_termios)) {
		close(serial_stream->fd);
		return UART_ERR_SERIAL_PORT_OPEN;
	}
	serial_port_flush();
	return UART_ERR_NONE;

}

UART_errCode serial_port_close(void) {
	#if DEBUG  > 1
		printf("Entering serial_port_close\n");
	#endif

	/* if null pointer or file descriptor indicates error just bail */
	if (!serial_stream || serial_stream->fd < 0)
		return UART_ERR_SERIAL_PORT_CLOSE;
	if (tcflush(serial_stream->fd, TCIOFLUSH)) {
		close(serial_stream->fd);
		return UART_ERR_SERIAL_PORT_CLOSE;
	}
	if (tcsetattr(serial_stream->fd, TCSADRAIN, &serial_stream->orig_termios)) {        // Restore modes.
		close(serial_stream->fd);
		return UART_ERR_SERIAL_PORT_CLOSE;
	}
	if (close(serial_stream->fd)) {
		return UART_ERR_SERIAL_PORT_CLOSE; 
	}
	
	serial_port_free();
	
	return UART_ERR_NONE;

}

uint8_t serial_port_get_length(void){
	#if DEBUG  > 1
		printf("Entering serial_port_get_length\n");
	#endif
	
	int i = 0;
	int serial_input_buffer_chars =0;
	int flag = 0;
	
	serial_input_buffer_clear();
	
	while(serial_input.buffer[0] != 0x99){
	    wait_for_data(); 
		ioctl(serial_stream->fd, FIONREAD, &serial_input_buffer_chars);        //set bytes to number of bytes in buffer
		if (serial_input_buffer_chars>0){
			serial_port_read(1);
		}	
	}

	serial_input_buffer_chars =serial_port_read(1);


	return serial_input.buffer[0];
}
	

UART_errCode serial_port_setup(void)
{
	#if DEBUG  > 1
		printf("Entering serial_port_setup\n");
	#endif
	if(serial_port_create()==UART_ERR_SERIAL_PORT_CREATE)
	{
		return UART_ERR_SERIAL_PORT_CREATE;
	}
	
	if(serial_port_open_raw(device, speed)==UART_ERR_SERIAL_PORT_OPEN)
	{
		return UART_ERR_SERIAL_PORT_OPEN;
	} 
	return UART_ERR_NONE;
}

int serial_port_get_baud() 
{
	#if DEBUG  > 1
		printf("Entering serial_port_get_baud\n");
	#endif
	
	struct termios termAttr;
	int inputSpeed = -1;
	speed_t baudRate;
	tcgetattr(serial_stream->fd, &termAttr);
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

UART_errCode serial_port_create()
{
	#if DEBUG  > 1
		printf("Entering serial_port_create\n");
	#endif
	
	char  tmp[256]={0x0};
	char flag = 0;
	FILE *fp ;
	int fd;

	fp = fopen("/sys/devices/bone_capemgr.9/slots", "r");
	if (fp == NULL){
		return UART_ERR_SERIAL_PORT_CREATE;
	} 


	while(flag!=1 && fp!=NULL && fgets(tmp, sizeof(tmp), fp)!=NULL)
	{
		if (strstr(tmp, "enable-uart5"))
		{
			flag = 1;
		}
	}
	#if DEBUG
	
	if(flag)
	{
		printf("Uart5 is enabled\n");
	} else {
		printf("Uart5 is disabled\n");
	}
	#endif
	
	fclose(fp);

	if (flag)
	{
		return UART_ERR_NONE;
	} else {
		fd = open("/sys/devices/bone_capemgr.9/slots", O_RDWR);
		
		#if DEBUG
			printf("Uart5 not enabled, trying to enable...\n");
	    #endif

		if (write(fd,"enable-uart5", 12)<0)
		{
			close(fd);
			return UART_ERR_SERIAL_PORT_CREATE;
		} else {
			close(fd);
			return UART_ERR_NONE;
		}
	}
	return UART_ERR_NONE;
}

int serial_port_read(uint32_t length) 
{
	#if DEBUG  > 1
		printf("Entering serial_port_read\n");
	#endif
	 
	int n = read(serial_stream->fd, serial_input.buffer, length);

	if (n < 1) 
	{
		return UART_ERR_READ;
	}                    
	return n;
}

UART_errCode serial_port_write(uint8_t output[],long unsigned int message_length) 
{
	#if DEBUG  > 1
		printf("Entering serial_port_write\n");
	#endif

	int n = write(serial_stream->fd, output, message_length);
		
	if (n < 0) 
	{
		return UART_ERR_SERIAL_PORT_WRITE;
	}
	return UART_ERR_NONE;                                                                                                           
}

void serial_buffer_clear(void)
{
	#if DEBUG  > 1
		printf("Entering serial_buffer_clear\n");
	#endif
	serial_input_buffer_clear();
}

void serial_input_buffer_clear(void)
{
	#if DEBUG  > 1
		printf("Entering serial_input_buffer_clear\n");
	#endif
	int i;
	for(i=0;i<serial_input_buffer_size;i++)
	{
		serial_input.buffer[i]=0x00;
	}
}


void benchmark_start(int timer)
{
	#if DEBUG  > 1
		printf("Entering benchmark_start\n");
	#endif
	
#if DEBUG > 1
	if(timer<10 && timer >-1)
	{
		gettimeofday(&timers[timer], NULL);
	}
#endif
}

void benchmark_stop(int timer)
{
	#if DEBUG  > 1
		printf("Entering benchmark_stop\n");
	#endif
	
#if DEBUG > 1 
	struct timeval current_time; 
	struct benchmark {
		uint32_t seconds;
		uint32_t mseconds;
		uint32_t useconds;
	};

	struct benchmark bench;

	if(timer<10 && timer >-1)
	{
		gettimeofday(&current_time, NULL);

		bench.seconds = current_time.tv_sec - timers[timer].tv_sec;
		
		bench.mseconds = (current_time.tv_usec - timers[timer].tv_usec)/1000;
		if (bench.mseconds < 0)
		{
			bench.seconds--;
			bench.mseconds +=1000;
		}

		bench.useconds = (current_time.tv_usec - timers[timer].tv_usec)-(bench.mseconds*1000);
		if (bench.useconds < 0)
		{
			bench.mseconds--;
			bench.useconds +=1000;
		}	
		if (bench.mseconds < 0)
		{
			bench.seconds--;
			bench.mseconds +=1000;
		}
		printf("timer %d: %ds %dms %dus ", timer, bench.seconds, bench.mseconds, bench.useconds);
	} else {
		printf("Timer %d is not started! Timer 0-9 available", timer);
	}
#endif
}
