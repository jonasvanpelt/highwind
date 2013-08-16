 #!/bin/bash
clear
gcc read_log.c udp_communication.c data_decoding.c -o read_log -DDEBUG 


./read_log 0 10.33.136.11 8888

