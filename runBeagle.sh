 #!/bin/bash
clear
gcc main_beaglebone.c udp_communication.c uart_communication.c log.c circular_buffer.c data_decoding.c  -pthread -DDEBUG=0 -o main_beaglebone

./main_beaglebone 10.33.136.11 8888 7777
