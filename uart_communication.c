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

#define DEBUG 2 

//#define TRACE(type,fmt,args...)    fprintf(stderr, fmt, args)
#define TRACE(type,fmt,args...)
#define TRACE_ERROR 1


//config
speed_t speed = B57600;
const char device[]="/dev/ttyO4";
int serial_input_buffer_size = sizeof(serial_input.buffer);
int serial_output_buffer_size = sizeof(serial_output.buffer);

/**
 * FUNCTIONS
 * */

int serial_input_check()
{
	int i;
	int serial_input_buffer_chars = 0;
	uint8_t checksum_1;
	uint8_t checksum_2;

	benchmark_start(0);
	usleep((useconds_t) 1); // short sleep to slow the loop down
	//Find number of bytes in buffer and read when enough

	ioctl(serial_stream->fd, FIONREAD, &serial_input_buffer_chars);        //set bytes to number of bytes in buffer
	if(serial_input_buffer_chars > 13)//tweak this number to gain performance - lower equals faster read but more likely to have erroneous strings
	{
		serial_input.buffer[0] = 0x00;
		usleep((useconds_t) 10); // short sleep in while loop
		serial_input_buffer_chars = serial_port_read(); //reads the port out and stores the number of chars red

#if DEBUG > 0
		if (serial_input_buffer_chars == -1) 
		{
			printf("read failed - serial_input_buffer: %s\n", serial_input.buffer);
		} else {
			checksum_1 = 0;
			checksum_2 = 0;
			for(i=1;i<serial_input.buffer[1]-2;i++)
			{
				checksum_1 += (uint8_t) serial_input.buffer[i];
				checksum_2 += checksum_1;
			}

			if (serial_input.converted.start != 0x99 || serial_input.converted.checksum_1 != checksum_1 || serial_input.converted.checksum_2 != checksum_2)
			{
				serial_port_flush_input();
#if DEBUG > 0
				packets.serial.lost++;
#endif
				return -1;
			} else {
#if DEBUG > 0
				packets.serial.received++;
				printf("start: %X ", serial_input.converted.start);
				printf("length: %d ", serial_input.converted.length);
				printf("checksum_1: %d ", serial_input.converted.checksum_1);
				printf("checksum_2: %d ", serial_input.converted.checksum_2);
				printf("checksum 1 calc: %d ", checksum_1);
				printf("checksum 2 calc: %d ", checksum_2);
				printf("lost / received: %d / %d ", packets.serial.lost, packets.serial.received);
#endif
				benchmark_stop(0);
				printf("\n");
				return 1;
			}
		}
#endif
	} else {
		return 0;
	}
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
		TRACE(TRACE_ERROR,"%s, set term attr failed: %s (%d)\n", device, strerror(errno), errno);
		fprintf(stderr, "flush (%d) failed: %s (%d)\n", serial_stream->fd, strerror(errno), errno);
	}
}

void serial_port_flush_output(void) {
	/*
	 * flush any input that might be on the port so we start fresh.
	 */
	if (tcflush(serial_stream->fd, TCOFLUSH)) {
		TRACE(TRACE_ERROR,"%s, set term attr failed: %s (%d)\n", device, strerror(errno), errno);
		fprintf(stderr, "flush (%d) failed: %s (%d)\n", serial_stream->fd, strerror(errno), errno);
	}
}

int  serial_port_open_raw(const char* device, speed_t speed) {
	if ((serial_stream->fd = open(device, O_RDWR | O_NONBLOCK | O_NOCTTY)) < 0) {
		TRACE(TRACE_ERROR,"%s, open failed: %s (%d)\n", device, strerror(errno), errno);
		return 0;
	}
	if (tcgetattr(serial_stream->fd, &serial_stream->orig_termios) < 0) {
		TRACE(TRACE_ERROR,"%s, get term settings failed: %s (%d)\n", device, strerror(errno), errno);
		close(serial_stream->fd);
		return 0;
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
		TRACE(TRACE_ERROR,"%s, set term speed failed: %s (%d)\n", device, strerror(errno), errno);
		close(serial_stream->fd);
		return 0;
	}
	if (tcsetattr(serial_stream->fd, TCSADRAIN, &serial_stream->cur_termios)) {
		TRACE(TRACE_ERROR,"%s, set term attr failed: %s (%d)\n", device, strerror(errno), errno);
		close(serial_stream->fd);
		return 0;
	}
	serial_port_flush();
	return 1;
}

int  serial_port_open(const char* device, void(*term_conf_callback)(struct termios*, speed_t*)) {

	speed_t speed;
	if ((serial_stream->fd = open(device, O_RDWR | O_NONBLOCK)) < 0) {
		TRACE(TRACE_ERROR,"%s, open failed: %s (%d)\n", device, strerror(errno), errno);
		return -1;
	}
	if (tcgetattr(serial_stream->fd, &serial_stream->orig_termios) < 0) {
		TRACE(TRACE_ERROR,"%s, get term settings failed: %s (%d)\n", device, strerror(errno), errno);
		close(serial_stream->fd);
		return -1;
	}
	serial_stream->cur_termios = serial_stream->orig_termios;
	term_conf_callback(&serial_stream->cur_termios, &speed);
	if (cfsetispeed(&serial_stream->cur_termios, speed)) {
		TRACE(TRACE_ERROR,"%s, set term speed failed: %s (%d)\n", device, strerror(errno), errno);
		close(serial_stream->fd);
		return -1;
	}
	if (tcsetattr(serial_stream->fd, TCSADRAIN, &serial_stream->cur_termios)) {
		TRACE(TRACE_ERROR,"%s, set term attr failed: %s (%d)\n", device, strerror(errno), errno);
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
		TRACE(TRACE_ERROR,"flushing (%s) (%d)\n", strerror(errno), errno);
		close(serial_stream->fd);
		return;
	}
	if (tcsetattr(serial_stream->fd, TCSADRAIN, &serial_stream->orig_termios)) {        // Restore modes.
		TRACE(TRACE_ERROR,"restoring term attributes (%s) (%d)\n", strerror(errno), errno);
		close(serial_stream->fd);
		return;
	}
	if (close(serial_stream->fd)) {
		TRACE(TRACE_ERROR,"closing %s (%d)\n", strerror(errno), errno);
		return;
	}
	return;
}

int serial_port_setup(void)
{
	if(!serial_port_create())
	{
		return 0;
	}
#if DEBUG>0
	printf("Opening serial stream... ");
#endif

	if(!serial_port_open_raw(device, speed))
	{
#if DEBUG>0
		printf("Unqble to open stream");
#endif
		perror("open_port: Unable to open /dev/ttyO4 - ");
		return 0;
	} else {
#if DEBUG>0
		printf("Serial connection established on /dev/ttyO4 with speed: %d bps\n", serial_port_get_baud());
#endif
	}
	return 1;
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
#if DEBUG>0
	printf("Opening /sys/devices/bone_capemgr.9/slots... ");
#endif
	fp = fopen("/sys/devices/bone_capemgr.9/slots", "r");
#if DEBUG>0
	if (fp != NULL){
		printf("opened \n");
	} else {
		printf("Unable to open the file\n");
	}
	printf("Searching the file to check if uart5 is enabled... ");
#endif
	if (fp == NULL)
	{
		return 0;
	}

	while(flag!=1 && fp!=NULL && fgets(tmp, sizeof(tmp), fp)!=NULL)
	{
		if (strstr(tmp, "enable-uart5"))
		{
			flag = 1;
		}
	}
#if DEBUG>0
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
		return 1;
	} else {
		fd = open("/sys/devices/bone_capemgr.9/slots", O_RDWR);
#if DEBUG>0
		printf("Uart5 not enabled, trying to enable...\n");
#endif
		if (write(fd,"enable-uart5", 12)<0)
		{
#if DEBUG>0
			printf("Unable to enable Uart5 on device\n");
#endif
			close(fd);
			return 0;
		} else {
#if DEBUG>0
			printf("Enabled Uart5 on device\n");
#endif
			close(fd);
			return 1;
		}
	}
}

int serial_port_read() 
{
	int n = read(serial_stream->fd, serial_input.buffer, serial_input_buffer_size);
	serial_input.buffer[n] = 0x00;                //This is needed or the previous contents of the string will appear after the changed characters. 
	serial_port_flush_input();
	if (n < 1) 
	{
		return -1;
	}                    
	return n;
}

int serial_port_write() 
{
	//chars[len] = 0x0d; // stick a after the command        (0xd == 13 == ASCII CR)
	serial_output.buffer[serial_output_buffer_size+1] = 0x00; // terminate the string properly
	int n = write(serial_stream->fd, serial_output.buffer, serial_output_buffer_size);
	if (n < 0) 
	{
		fputs("write failed!\n", stderr);
		return 0;
	}
	return 1;                                                                                                           
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
