#ifndef UDP_COMMUNCATION_H_ 
#define UDP_COMMUNCATION_H_

#include<sys/socket.h>
#include<arpa/inet.h>

typedef struct 
{ 
	struct sockaddr_in si_other;
	int fd;
	int fd_len;
} UDP_client;

typedef struct 
{ 
	struct sockaddr_in si_me;
	struct sockaddr_in si_other;
	int fd;
	int fd_len;
} UDP_server;


//udp client prototypes
extern void openUDPClientSocket(UDP_client *udp_client,char *server_ip,int port);
extern void sendUDPClientData(UDP_client *udp_client,void *data,size_t data_len);
extern void closeUDPClientSocket(UDP_client *udp_client);

//udp server prototypes
extern void openUDPServerSocket(UDP_server *udp_server,int port);
extern void receiveUDPServerData(UDP_server *udp_server,void *data,size_t data_len);
extern void closeUDPServerSocket(UDP_server *udp_server);

#endif /*UDP_COMMUNCATION_H__*/

