 #!/bin/bash
clear

gcc main_windsensor.c udp_communication.c uart_communication.c log.c circular_buffer.c data_decoding.c -DDEBUG=0 -o main_wind

# Jonas
./main_wind 127.0.0.1 8888


