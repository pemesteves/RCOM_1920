#include "ftp.h"

int ftp_connect_server(FTP* ftp, const char* ip, int port) {
    int sockfd;
	struct sockaddr_in server_addr;
	int	bytes;
	
	// server address handling
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);	// 32 bit Internet address network byte ordered
	server_addr.sin_port = htons(port);		        //server TCP port must be network byte ordered 
    
	// open an TCP socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(0);
    }

	// connect to the server
    if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("connect()");
        exit(0);
	}

    ftp->socket_fd = sockfd;
}
