/*
 * AUTHOR: Maarten Arits and Jonas Van Pelt
 */

#ifndef UART_COMMUNCATION_H_ 
#define UART_COMMUNCATION_H_

#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>

#define INPUT_BUFFER_SIZE 255 

/********************************
 * GLOBALS
 * ******************************/
 
enum uart_errCode {UART_ERR_READ= -6 ,UART_ERR_READ_START_BYTE = -5,UART_ERR_READ_CHECKSUM = -4,UART_ERR_READ_LENGTH = -3,UART_ERR_READ_MESSAGE = -2, UART_ERR_NONE=0,UART_ERR_SERIAL_PORT_FLUSH_INPUT,UART_ERR_SERIAL_PORT_FLUSH_OUTPUT,UART_ERR_SERIAL_PORT_OPEN,UART_ERR_SERIAL_PORT_CLOSE,UART_ERR_SERIAL_PORT_CREATE,UART_ERR_SERIAL_PORT_WRITE,UART_ERR_UNDEFINED};
typedef enum uart_errCode UART_errCode;
 
typedef struct{
	int fd;                        /* serial device fd          */
	struct termios orig_termios;   /* saved tty state structure */
	struct termios cur_termios;    /* tty state structure       */
}serial_port; 
 
serial_port *serial_stream;


/********************************
 * PROTOTYPES PUBLIC
 * ******************************/
 
extern UART_errCode serial_port_setup(void); 
extern int serial_input_get_data(uint8_t buffer[]); //returns the number of read bytes or a negative error message and puts the result in serial_input
extern UART_errCode serial_port_write(uint8_t output[],long unsigned int message_length) ;
extern UART_errCode serial_port_close(void);


#endif /*UART_COMMUNCATION_H_*/
