#include <stdio.h>
#include <stdlib.h>

#include "uart_communication.h"

int main(int argc, char *argv[])
{
	serial_stream=serial_port_new();

	system("clear"); // clear the screen

	packets_clear();

	if (!serial_port_setup())
	{
#if DEBUG > 0
		printf("Setup has failed, port couldn't be opened\n");
#endif
		return 1;
	}

	while(1)
	{
		//serial_port_write();
		serial_input_check();
		serial_input_buffer_clear();
	}

	serial_port_close();
	serial_port_free();
	return 0;
}
