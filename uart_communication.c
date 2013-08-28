
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

#include "header_files/uart_communication.h"

#ifndef DEBUG 
#define DEBUG 0
#endif


/********************************
 * PROTOTYPES PRIVATE
 * ******************************/
 
static int serial_port_read(uint8_t buffer[],int length); 
static UART_errCode serial_port_new(void);
static UART_errCode serial_port_create();
static UART_errCode  serial_port_open_raw(const char* device, speed_t speed);
//static UART_errCode  serial_port_open(const char* device, void(*term_conf_callback)(struct termios*, speed_t*));
static void serial_port_free(void);
static void serial_port_flush(void);
static UART_errCode serial_port_flush_input(void);
static UART_errCode serial_port_flush_output(void);
static void packets_clear(void);
static int wait_for_data();

/********************************
 * GLOBALS
 * ******************************/
 
 
static const char FILENAME[] = "uart_communication.c";

//config
speed_t speed = B921600;
const char device[]="/dev/ttyO4";

/********************************
 * FUNCTIONS
 * ******************************/

static int wait_for_data(){
	struct pollfd fds[1];
	int timeout = 1000000; //time out in millisecond
	int result;
	fds[0].fd=serial_stream->fd;
	fds[0].events=POLLIN;
	result=poll(fds,1,timeout); //block until there is data in the serial stream

	if((result & (1 << 0)) == 0){
		return -1;	
	}
	return 0;
}

static int serial_port_read(uint8_t buffer[],int length) 
{
	#if DEBUG  > 1
		printf("Entering serial_port_read\n");
	#endif
	
	int bytes_in_buffer=0;

	do{
		wait_for_data();
		ioctl(serial_stream->fd, FIONREAD, &bytes_in_buffer); //set to number of bytes in buffer
		printf("bytes in buff %d\n",bytes_in_buffer);
	
	}while(bytes_in_buffer < length );
	 
	int n = read(serial_stream->fd, buffer, length);
	 
	if(n==-1){
		return UART_ERR_READ;
	}  
	    
	return n;  //return number of read bytes
}

int serial_input_get_data(uint8_t buffer[]){
	
	//todo: oneindige loops vermijden
	
	uint8_t message_length;
	int bytes_in_buffer;
	uint8_t checksum_1=0;
	uint8_t checksum_2=0;
	int i;
	int INDEX_START_BYTE=0,INDEX_LENGTH=1,INDEX_CH1,INDEX_CH2;

	//1. SEARCH FOR START BYTE
	do{
		if(serial_port_read(&buffer[0],1)==UART_ERR_READ){	//read first byte
			return UART_ERR_READ_START_BYTE;
		} 
	
	}while(buffer[0]!=0x99);

	//buffer[0] = 0x99 at this moment
	
	//2. READ MESSAGE LENGTH
	if(serial_port_read(&buffer[1],1)==UART_ERR_READ){	//read first byte
			return UART_ERR_READ_LENGTH;
	} 
	message_length = buffer[1]; 
	//buffer[1] = length at this moment

	
	//3. READ MESSAGE
	if(serial_port_read(&buffer[2],message_length-2)==UART_ERR_READ){	//read only message_length -2 because start byte and length are already in there
			return UART_ERR_READ_MESSAGE;
	} 
	
	//4 CHECK CHECKSUMS
	INDEX_CH1 = message_length-2;
	INDEX_CH2 = message_length-1;
	
	for(i=1;i<message_length-2;i++) //read until message_length - checksum_1 - checksum_2
	{
		checksum_1 += buffer[i];
		checksum_2 += checksum_1;
	}
	
	if (buffer[INDEX_CH1]!= checksum_1 || buffer[INDEX_CH2] != checksum_2)
	{
		return UART_ERR_READ_CHECKSUM; 
	}
	
	printf("message raw: ");
	for(i=0;i<message_length;i++){
			printf("%d ",buffer[i]);
	}
	printf("\n");
	
	return message_length;
}

 

static UART_errCode serial_port_new(void) {
	#if DEBUG  > 1
		printf("Entering serial_port_new\n");
	#endif
	
	serial_stream = (serial_port*) malloc(sizeof(serial_port));
	
	if(serial_stream==NULL){
			return UART_ERR_SERIAL_PORT_CREATE;
	}
	
	return UART_ERR_NONE;
}

static void serial_port_free(void) {
	#if DEBUG  > 1
		printf("Entering serial_port_free\n");
	#endif
	
	free(serial_stream);
}

static void serial_port_flush(void) {
	#if DEBUG  > 1
		printf("Entering serial_port_flush\n");
	#endif
	/*
	 * flush any input and output on the port
	 */
	serial_port_flush_input();
	serial_port_flush_output();
}


static UART_errCode serial_port_flush_input(void) {
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

static UART_errCode serial_port_flush_output(void) {
	
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

static UART_errCode  serial_port_open_raw(const char* device, speed_t speed) {
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

/*static UART_errCode  serial_port_open(const char* device, void(*term_conf_callback)(struct termios*, speed_t*)) {
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

}*/

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



UART_errCode serial_port_setup(void)
{
	#if DEBUG  > 1
		printf("Entering serial_port_setup\n");
	#endif
	
	int err;
	
	err = serial_port_new();
	if(err!=UART_ERR_NONE){
			return err;
	}
	
	err = serial_port_create();
	if(err!=UART_ERR_NONE){
			return err;
	}
	
	err = serial_port_open_raw(device, speed);
	if(err!=UART_ERR_NONE){
		return err;
	}
	
	return UART_ERR_NONE;
}

static UART_errCode serial_port_create()
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
