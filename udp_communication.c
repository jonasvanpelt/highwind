#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include "udp_communication.h"

void die(char *s)
{
    perror(s);
    exit(1);
}

void openUDPClientSocket(UDP_client *udp_client,char *server_ip,int port){
	
	//define fd_len
    udp_client->fd_len = sizeof(udp_client->si_other);
    
    //create a UDP socket
	if ( (udp_client->fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		die("socket");
	} 
	
	//zero si_other
	memset((char *) &(udp_client->si_other), 0,udp_client->fd_len);

	udp_client->si_other.sin_family = AF_INET;
	udp_client->si_other.sin_port = htons(port);
     
	/* inet_aton converts the Internet host address cp from the standard numbers-and-dots notation into binary data and stores it in the structure that inp points to. inet_aton returns nonzero if the address is valid, zero if not*/
	if (inet_aton(server_ip , &(udp_client->si_other.sin_addr)) == 0) 
	{
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}
}

void sendUDPClientData(UDP_client *udp_client,void *data,size_t data_len){
	//send the message
	if (sendto(udp_client->fd, (void *)data, data_len , 0 , (struct sockaddr *) &(udp_client->si_other), udp_client->fd_len)==-1)
	{
		die("sendto()");
	}
}

void closeUDPClientSocket(UDP_client *udp_client){
	close(udp_client->fd);
}



void openUDPServerSocket(UDP_server *udp_server,int port){
	
	//define fd_len
   	 udp_server->fd_len = sizeof(udp_server->si_other);

	//create a UDP socket
	if ((udp_server->fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		die("socket");
	}

	// zero out the structure
	memset((char *) &(udp_server->si_me), 0, sizeof(udp_server->si_me));

	udp_server->si_me.sin_family = AF_INET;
	udp_server->si_me.sin_port = htons(port);
	udp_server->si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	//bind socket to port
	if( bind(udp_server->fd , (struct sockaddr*)&(udp_server->si_me), sizeof(udp_server->si_me) ) == -1)
	{
		die("bind");
	}
}

void receiveUDPServerData(UDP_server *udp_server,void *data,size_t data_len){
	int recv_len;
	//blocking !!!
	if ((recv_len = recvfrom(udp_server->fd, data,data_len, 0, (struct sockaddr *) &(udp_server->si_other), &(udp_server->fd_len))) == -1)
	{
		die("recvfrom()");
	}else{
		//eventually send something back to sender using si_other
	}
}


void closeUDPServerSocket(UDP_server *udp_server){
	close(udp_server->fd);
}


