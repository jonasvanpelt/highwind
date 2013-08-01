/*
 * AUTHOR: Maarten Arits
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <time.h>

#include "uart_communication.h"

/**
 * debug:
 *
 * 0: nog output on screen
 * 1: feedback on screen
 * 2: benchmarking on screen
 *
 * */

#ifndef DEBUG 
#define DEBUG 3
#endif

//#define TRACE(type,fmt,args...)    fprintf(stderr, fmt, args)
#define TRACE(type,fmt,args...)
#define TRACE_ERROR 1

static const char FILENAME[] = "uart.c";

//config
speed_t speed = B57600;
const char device[]="/dev/ttyO4";
int serial_input_buffer_size = sizeof(serial_input.buffer);
int serial_output_buffer_size = sizeof(serial_output.buffer);

/**
 * FUNCTIONS
 * */

int serial_input_check() //returns the number of red bytes
{
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

	ioctl(serial_stream->fd, FIONREAD, &serial_input_buffer_chars);        //set bytes to number of bytes in buffer
	if(serial_input_buffer_chars > 0) //look for char in buffer
	{
		
		/*serial_port_read(1);
		for(i=0;i<1;i++){
			printf("%x ",serial_input.buffer[i]);
		}
		printf("\n");	*/
		message_length = serial_port_get_length();

		if(message_length == 0){
			error_write(FILENAME,"serial_input_check()","read failed - message_length is zero");
		}

		gettimeofday(&start, NULL);
		
		serial_input_buffer_chars=0;
		while(serial_input_buffer_chars<message_length-2){
			ioctl(serial_stream->fd, FIONREAD, &serial_input_buffer_chars);        
			usleep(5);
		}

		serial_input_buffer_chars = serial_port_read(message_length-2); //reads the port out and stores the number of chars red

		for(i=0;i<message_length-2;i++){
			printf("%d ",serial_input.buffer[i]);
		}
		printf("\n");
				
		
		if (serial_input_buffer_chars == -1) 
		{
			error_write(FILENAME,"serial_input_check()","read failed - serial_input_buffer");

		} else {

			checksum_1 = message_length; //part of the checksum
			checksum_2 = checksum_1; // add checksum_0 to 0

			for(i=0;i<message_length-4;i++) // count until checksum 1 --> length - 2 (checksums) - 2 (ofset)
			{
				checksum_1 += (uint8_t) serial_input.buffer[i];
				checksum_2 += checksum_1;
			}

			/*printf("check1 %d\n",serial_input.buffer[message_length-4]);
			printf("check2 %d\n",serial_input.buffer[message_length-3]);
			printf("check %d\n",checksum_1);
			printf("check %d\n",checksum_2);*/


			if (serial_input.buffer[message_length-4]!= checksum_1 || serial_input.buffer[message_length-3] != checksum_2)
			{
				serial_port_flush_input();
				packets.serial.lost++;
				return -1;

			} else {

				//first two bits (start and length ) should be in buffer, now
				char temp[INPUT_BUFFER];
				int i;
				for(i=0;i<INPUT_BUFFER;i++)
				{
					temp[i]=serial_input.buffer[i];	
				}
				serial_input.buffer[0]=0x99;
				serial_input.buffer[1]=message_length;
				for(i=2;i<INPUT_BUFFER;i++)
				{
					serial_input.buffer[i]=temp[i];	
				}
				

				packets.serial.received++;

#if DEBUG > 0
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
		return -1;
	}
	return message_length;
}

void packets_clear(void)
{
	packets.serial.received=0;
	packets.serial.lost=0;
	packets.udp.received=0;
	packets.udp.lost=0;
}

serial_port* serial_port_new(void) {
	serial_port* serial_stream = (serial_port*) malloc(sizeof(serial_port));
	return serial_stream;
}

void serial_port_free(void) {
	free(serial_stream);
}

void serial_port_flush(void) {
	/*
	 * flush any input and output on the port
	 */
	serial_port_flush_input();
	serial_port_flush_output();
}


void serial_port_flush_input(void) {
	/*
	 * flush any input that might be on the port so we start fresh.
	 */
	if (tcflush(serial_stream->fd, TCIFLUSH)) {
		error_write(FILENAME,"serial_port_flush_input()","flush input failed");

	}
}

void serial_port_flush_output(void) {
	/*
	 * flush any input that might be on the port so we start fresh.
	 */
	if (tcflush(serial_stream->fd, TCOFLUSH)) {
			error_write(FILENAME,"serial_port_flush_output()","flush output failed");

	}
}

int  serial_port_open_raw(const char* device, speed_t speed) {
	if ((serial_stream->fd = open(device, O_RDWR | O_NONBLOCK | O_NOCTTY)) < 0) {
			error_write(FILENAME,"serial_port_open_raw()","opening serial port failed");
		return -1;
	}
	if (tcgetattr(serial_stream->fd, &serial_stream->orig_termios) < 0) {
		error_write(FILENAME,"serial_port_open_raw()","get term settings failed");
		close(serial_stream->fd);
		return -1;
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
		error_write(FILENAME,"serial_port_open_raw()","set term attr failed");
		close(serial_stream->fd);
		return -1;
	}
	if (tcsetattr(serial_stream->fd, TCSADRAIN, &serial_stream->cur_termios)) {
		error_write(FILENAME,"serial_port_open_raw()","set term attr failed");
		close(serial_stream->fd);
		return -1;
	}
	serial_port_flush();
	return 0;
}

int  serial_port_open(const char* device, void(*term_conf_callback)(struct termios*, speed_t*)) {

	speed_t speed;
	if ((serial_stream->fd = open(device, O_RDWR | O_NONBLOCK)) < 0) {
			error_write(FILENAME,"serial_port_open()","opening serial port failed");
		return -1;
	}
	if (tcgetattr(serial_stream->fd, &serial_stream->orig_termios) < 0) {
		error_write(FILENAME,"serial_port_open()","get term settings failed");
		close(serial_stream->fd);
		return -1;
	}
	serial_stream->cur_termios = serial_stream->orig_termios;
	term_conf_callback(&serial_stream->cur_termios, &speed);
	if (cfsetispeed(&serial_stream->cur_termios, speed)) {
		error_write(FILENAME,"serial_port_open()","set term speed failed");
		close(serial_stream->fd);
		return -1;
	}
	if (tcsetattr(serial_stream->fd, TCSADRAIN, &serial_stream->cur_termios)) {
		TRACE(TRACE_ERROR,"%s, set term attr failed: %s (%d)\n", device, strerror(errno), errno);
		error_write(FILENAME,"serial_port_open()","set term attr failed");
		close(serial_stream->fd);
		return -1;
	}
	serial_port_flush();
	return 0;

}

void serial_port_close(void) {

	/* if null pointer or file descriptor indicates error just bail */
	if (!serial_stream || serial_stream->fd < 0)
		return;
	if (tcflush(serial_stream->fd, TCIOFLUSH)) {
		//TRACE(TRACE_ERROR,"flushing (%s) (%d)\n", strerror(errno), errno);
		close(serial_stream->fd);
		return;
	}
	if (tcsetattr(serial_stream->fd, TCSADRAIN, &serial_stream->orig_termios)) {        // Restore modes.
		//TRACE(TRACE_ERROR,"restoring term attributes (%s) (%d)\n", strerror(errno), errno);
		close(serial_stream->fd);
		return;
	}
	if (close(serial_stream->fd)) {
		//TRACE(TRACE_ERROR,"closing %s (%d)\n", strerror(errno), errno);
		return;
	}
	return;
}

uint8_t serial_port_get_length(void){
	int i = 0;
	int serial_input_buffer_chars =0;
	int flag = 0;
	
	serial_input_buffer_clear();
	
	while(serial_input.buffer[0] != 0x99){
		ioctl(serial_stream->fd, FIONREAD, &serial_input_buffer_chars);        //set bytes to number of bytes in buffer
		if (serial_input_buffer_chars>0){
			serial_port_read(1);
		}	
		usleep(20);
	}
	usleep(25);

	serial_input_buffer_chars =serial_port_read(1);

	if (serial_input_buffer_chars == -1) 
	{
		error_write(FILENAME,"serial_port_get_length()","read failed - serial_input_buffer");

	}

	return serial_input.buffer[0];
}
	

int serial_port_setup(void)
{
	if(serial_port_create()==-1)
	{
		return -1;
	}
	
	log_write(FILENAME,"serial_port_setup()","Opening serial stream...");

	if(serial_port_open_raw(device, speed)==-1)
	{
		error_write(FILENAME,"serial_port_setup()","open_port: Unable to open /dev/ttyO4 ");
		return -1;
	} else {
		log_write(FILENAME,"serial_port_setup()","Serial connection established on /dev/ttyO4");
	}
	return 0;
}

int serial_port_get_baud() 
{
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

int serial_port_create()
{
	char  tmp[256]={0x0};
	char flag = 0;
	FILE *fp ;
	int fd;

	log_write(FILENAME,"serial_port_create()","Opening /sys/devices/bone_capemgr.9/slots... ");

	fp = fopen("/sys/devices/bone_capemgr.9/slots", "r");
	if (fp == NULL){
		error_write(FILENAME,"serial_port_create()","Unable to open the file");
	} 
	log_write(FILENAME,"serial_port_create()","Searching the file to check if uart5 is enabled... ");

	if (fp == NULL)
	{
		return -1;
	}

	while(flag!=1 && fp!=NULL && fgets(tmp, sizeof(tmp), fp)!=NULL)
	{
		if (strstr(tmp, "enable-uart5"))
		{
			flag = 1;
		}
	}
	if(flag)
	{
		log_write(FILENAME,"serial_port_create()","Uart5 is enabled");
	} else {
		log_write(FILENAME,"serial_port_create()","Uart5 is disabled");
	}

	fclose(fp);

	if (flag)
	{
		return 0;
	} else {
		fd = open("/sys/devices/bone_capemgr.9/slots", O_RDWR);
		
		log_write(FILENAME,"serial_port_create()","Uart5 not enabled, trying to enable...");

		if (write(fd,"enable-uart5", 12)<0)
		{
			error_write(FILENAME,"serial_port_create()","failed to enable Uart5 on device");
			close(fd);
			return -1;
		} else {
		    log_write(FILENAME,"serial_port_create()","Enabled Uart5 on device");	
			close(fd);
			return 0;
		}
	}
}

int serial_port_read(uint32_t length) 
{
	int n = read(serial_stream->fd, serial_input.buffer, length);
	//serial_input.buffer[n] = 0x00;                //This is needed or the previous contents of the string will appear after the changed characters. 
	//serial_port_flush_input();
	if (n < 1) 
	{
		return -1;
	}                    
	return n;
}

int serial_port_write() 
{
	//chars[len] = 0x0d; // stick a after the command        (0xd == 13 == ASCII CR)
	//serial_output.buffer[serial_output_buffer_size+1] = 0x00; // terminate the string properly
	int n = write(serial_stream->fd, serial_output.buffer, serial_output_buffer_size);
	if (n < 0) 
	{
		error_write(FILENAME,"serial_port_write()","serial port write failed");
		return -1;
	}
	return 0;                                                                                                           
}

void serial_buffer_clear(void)
{
	serial_input_buffer_clear();
	serial_output_buffer_clear();
}

void serial_output_buffer_clear(void)
{
	int i;
	for(i=0;i<serial_output_buffer_size;i++)
	{
		serial_output.buffer[i]=0x00;
	}
}


void serial_input_buffer_clear(void)
{
	int i;
	for(i=0;i<serial_input_buffer_size;i++)
	{
		serial_input.buffer[i]=0x00;
	}
}




void benchmark_start(int timer)
{
#if DEBUG > 1
	if(timer<10 && timer >-1)
	{
		gettimeofday(&timers[timer], NULL);
	}
#endif
}

void benchmark_stop(int timer)
{
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
