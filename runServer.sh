 #!/bin/bash
clear
gcc main_full_server.c log.c udp_communication.c data_decoding.c analyze.c -pthread -o main_server -DDEBUG=0 -DANALYZE=1


./main_server 10.33.136.55 7777 8888 
