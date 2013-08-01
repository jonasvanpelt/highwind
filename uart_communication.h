/*
 * AUTHOR: Maarten Arits
 */

#ifndef UART_COMMUNCATION_H_ 
#define UART_COMMUNCATION_H_

#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>

#define INPUT_BUFFER 255 
#define OUTPUT_BUFFER 34

/**
 * GLOBALS
 * */
typedef struct{
	int fd;                        /* serial device fd          */
	struct termios orig_termios;   /* saved tty state structure */
	struct termios cur_termios;    /* tty state structure       */
}serial_port; 
 
serial_port *serial_stream;


struct Packets {
	struct Serial {
		uint32_t received;
		uint32_t lost;
	} serial;
	struct UDP {
		uint32_t received;
		uint32_t lost;
	} udp;
} packets;

union Serial_input {
	char buffer[INPUT_BUFFER]; //must be set bigger
} serial_input;

union Serial_output {
	char buffer[OUTPUT_BUFFER];
	uint32_t set_servo_buffer[8];
} serial_output;


//timers

struct timeval timers[10];

/**
 * PROTOTYPES
 * */

extern int serial_port_setup(void);
extern int serial_input_check(void);
extern int serial_port_write(void);
extern int serial_port_read(uint32_t length);
extern int serial_port_create(void);
extern int serial_port_get_baud(void);
extern int  serial_port_open_raw(const char* device, speed_t speed);
extern int  serial_port_open(const char* device, void(*term_conf_callback)(struct termios*, speed_t*));
extern serial_port* serial_port_new(void);
extern void serial_port_free(void);
extern void serial_port_flush(void);
extern void serial_port_flush_input(void);
extern void serial_port_flush_output(void);
extern void serial_port_close(void);
extern void serial_buffer_clear(void);
extern void serial_output_buffer_clear(void);
extern void serial_input_buffer_clear(void);
extern void benchmark_start(int timer);
extern void benchmark_stop(int timer);
extern void packets_clear(void);
extern uint8_t serial_port_get_length(void);

#endif /*UART_COMMUNCATION_H_*/
