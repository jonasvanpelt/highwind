 #!/bin/bash
clear

gcc main_arm.c udp_communication.c spi_communication.c log.c data_decoding.c -DDEBUG=0 -o main_arm

# Jonas
./main_arm 10.33.136.11 8888


