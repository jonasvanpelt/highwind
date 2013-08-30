/*
 * * Copyright (C) 2013 Alan Backlund
 * *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 as
 * * published by the Free Software Foundation.
 * */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "header_files/SimpleGPIO.h"
#include "header_files/spi_communication.h"

int fd0;
int fd1;
const unsigned int status0 = 115;
const unsigned int status1 = 116;

SPI_errCode spi_open(){
	


	fd0 = open(spidev, O_RDWR);
	if (fd0 < 0) {
		return SPI_ERR_OPEN_DEV
	}	
	
		fd1 = open(spidev1, O_RDWR);
	if (fd1 < 0) {
		return SPI_ERR_OPEN_DEV
	}
	
	
	gpio_export(status0);
	gpio_export(status1);
	gpio_set_dir(status0, INPUT_PIN);
	gpio_set_dir(status1, INPUT_PIN);	
	
	
	return SPI_ERR_NONE;
}
SPI_errCode spi_read(uint8_t data_sensors[]){
	
		uint8_t cmd[1], rsp[2];
		cmd[0] = 0x22;

	    uint8_t cmd1[1], rsp1[1];
            cmd1[0] = 0x06;
            
            data_sensors[0]=2;
            data_sensors[1]=2;
            data_sensors[2]=2;
            data_sensors[3]=2;

	unsigned int value0 = HIGH;
	unsigned int value1 = HIGH;	    
	
	return SPI_ERR_NONE;
}
SPI_errCode spi_close(){
	return SPI_ERR_NONE;
}

void SPI_err_handler(SPI_errCode err,void (*write_error_ptr)(char *,char *,int)){
	//write error to local log
	switch( err ) {
		case SPI_ERR_NONE:
			break;
		case  SPI_ERR_UNDEFINED:
			write_error_ptr(SOURCEFILE,"undefined spi error",err);
			break;
		case  SPI_ERR_OPEN_DEV:
			write_error_ptr(SOURCEFILE,"failed to open spi port",err);
			break;
		default: break;
	}
}


int main(int argc, const char *argv[])
{
	gpio_export(status0);
	gpio_export(status1);
	gpio_set_dir(status0, INPUT_PIN);
	gpio_set_dir(status1, INPUT_PIN);

	uint8_t cmd[1], rsp[1];
		cmd[0] = 0X02;
   /* cmd[1] = 0x12;
		cmd[2] = 0x34;
	cmd[3] = 0x56;
	   */          
           
	    uint8_t cmd1[1], rsp1[1];
            cmd1[0] = 0X06;
	  /*  cmd1[1] = 0x16;
            cmd1[2] = 0x38;
	    cmd1[3] = 0x59;*/

	unsigned int value0 = HIGH;
	unsigned int value1 = HIGH;
            
	int i;

	for( i = 0; i < 10 ; i++){
	
	const char *spidev = "/dev/spidev1.0";
	if (argc == 2)
		spidev = argv[1];
	fd0 = open(spidev, O_RDWR);
	if (fd0 < 0) {
		perror("file open");
		abort();
	}

	struct spi_ioc_transfer tr;

	printf("Sensor 0\n");

	tr.tx_buf = (unsigned long)cmd;

	gpio_get_value(status0, &value0);

	if (value0 == HIGH){
		tr.rx_buf = (unsigned long)rsp;
	}
	else{
		tr.rx_buf = 0;
		rsp[0]=0;
		printf("Wrong Information\n");		  
		/*printf("%i\n",value0);*/

	}

	tr.len = 4;
	tr.delay_usecs = 0;
	tr.speed_hz = 2500000;
	tr.bits_per_word = 0;
	tr.cs_change = 1;


	int ret = ioctl(fd0, SPI_IOC_MESSAGE(1), &tr);

	if (ret < 0) {
		perror("ioctl");
	}

	printf("cmd: 0x%02X\n", cmd[0]);

	printf("rsp: 0x%02X\n\n", rsp[0]);        

	cmd[0]++;

	close(fd0);


	const char *spidev1 = "/dev/spidev1.1";
		if (argc == 2)
		spidev1 = argv[1];
		fd1 = open(spidev1, O_RDWR);
	if (fd1 < 0) {
		perror("file open");
		abort();
	}





	struct spi_ioc_transfer tr1;

	printf("Sensor 1\n");

	tr1.tx_buf = (unsigned long)cmd1;

	gpio_get_value(status1, &value1);

	if (value1 == HIGH){
	tr1.rx_buf = (unsigned long)rsp1;
	}		 

	else {		
	tr1.rx_buf = 0;
	rsp1[0]=0;
	printf("Wrong Information\n");		  
	/*printf("%i\n",value1);*/


	}

	tr1.len = 4;
	tr1.delay_usecs = 0;
	tr1.speed_hz = 2500000;
	tr1.bits_per_word = 0;
	tr1.cs_change = 1;

	int ret1 = ioctl(fd1, SPI_IOC_MESSAGE(1), &tr1);

	if (ret1 < 0) {

	perror("ioctl");

	}

	printf("cmd1: 0x%02X\n", cmd1[0]);

	printf("rsp1: 0x%02X\n\n", rsp1[0]);


	close(fd1); 
		};
		
	gpio_unexport(status0);
	gpio_unexport(status1);

	   
}

