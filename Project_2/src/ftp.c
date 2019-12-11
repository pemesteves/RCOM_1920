#include "ftp.h"
#include <stdbool.h>
#include <string.h>

int ftp_create_socket(int* socket_fd, const char* ip, int port) {
	struct sockaddr_in server_addr;
	int	bytes;
	
	// server address handling
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);	// 32 bit Internet address network byte ordered
	server_addr.sin_port = htons(port);		        //server TCP port must be network byte ordered 
    
	// open an TCP socket
	if ((*socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        return -1;
    }

	// connect to the server
    if(connect(*socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("connect()");
        return -1;
	}

	return 0;
}

int ftp_connect_server(FTP* ftp) {
    if(ftp_create_socket(&(ftp->socket_fd), ftp->url.ip, ftp->url.port) < 0)
		return -1; 

	int reply_code = ftp_server_reply(ftp, NULL);
	
	switch(reply_code) {
		case SERVER_READY:
			printf("Server is ready. Connection estabilished.\n");
			break;

		case SERVER_BUSY:
			printf("Server is busy. Waiting...\n");
			reply_code = ftp_server_reply(ftp, NULL);
			if(reply_code != SERVER_READY) {
				printf("Unable to connect\n");
				return -1;
			}
			printf("Connection estabilished.\n");
			break;

		default:
			printf("Unable to connect\n");
			return -1;
	}
}

void ftp_close_server(FTP* ftp) {
	close(ftp->socket_fd);
	close(ftp->data_socket_fd);
}

int ftp_command(FTP* ftp, const char* cmd, const char* arg, char* reply) {
	
	char buffer[1024];

	sprintf(buffer, "%s %s\r\n", cmd, arg);
	write(ftp->socket_fd, buffer, strlen(buffer));

	return ftp_server_reply(ftp, reply);
}

int ftp_server_reply(FTP* ftp, char* reply) {
	// flags
	bool received_reply = false;
	bool multi_line = false;

	// buffers
	char buffer[2048];
	char reply_code[4];
	reply_code[3] = '\0';

	// reads the reply code
	read(ftp->socket_fd, reply_code, 3);

	// determines if it's a multi-line reply
	read(ftp->socket_fd, buffer, 1);
	if(buffer[0] == '-')
		multi_line = true;
	else if(buffer[0] != ' ')
		return -1;

	int reply_size = 0;

	while(!received_reply) {
		ssize_t bytes_read = read(ftp->socket_fd, buffer, 2048);

		if(multi_line) {
			for(unsigned index = 0; index < bytes_read; index++) {

				// determines if reply code was found again
				if(memcmp(reply_code, buffer + index, 3)== 0 && buffer[index + 3] == ' ') {
					multi_line = false;
					received_reply = true;
				}
			}
		}

		// single line
		else {
			if(buffer[bytes_read - 2] == '\r' && buffer[bytes_read - 1] == '\n')
				received_reply = true;
		}

		if(reply != NULL) {
			memcpy(reply + reply_size, buffer, bytes_read);
			reply_size += bytes_read;
		}
	}

	return atoi(reply_code);
}

int ftp_user_command(FTP* ftp, char* reply) {
	int reply_code = ftp_command(ftp, "USER", ftp->url.user, reply);

	switch(reply_code) {
		case PASSWORD_NEEDED:
			printf("User name okay, need password.\n\t%d - %s\n", reply_code, reply);
			return 0;

		case USER_LOGGED:
			printf("User logged in.\n");
			return 1;

		default:
			printf("Unexpected reply code.\n\t%d - %s\n", reply_code, reply);
			return -1;
	}
}

int ftp_pass_command(FTP* ftp, char* reply) {
	int reply_code = ftp_command(ftp, "PASS", ftp->url.password, reply);

	switch(reply_code) {
		case USER_LOGGED:
			printf("User logged in.\n");
			return 0;

		case NOT_LOGGED_IN:
			printf("Not logged in.\n\t%d - %s\n", reply_code, reply);
			return -2;

		case BAD_SEQUENCE:
			printf("Bad sequence of commands.\n\t%d - %s\n", reply_code, reply);
			return -1;

		default:
			printf("Unexpected reply code.\n\t%d - %s\n", reply_code, reply);
			return -1;
	}
}

int ftp_cwd_command(FTP* ftp, char* reply) {
	if(strlen(ftp->url.path)==0) {
		printf("Already at correct directory\n");
		return 0;
	}
		
	int reply_code = ftp_command(ftp, "CWD", ftp->url.path, reply);

	printf("DIRECTORY: %s\n\n", ftp->url.path);

	if(reply_code != FILE_ACTION_OK) {
		printf("Unexpected reply code. Probably a wrong directory\n");
		return -1;
	}

	printf("Successfuly changed directory\n");
	return 0;
}

int ftp_pasv_command(FTP* ftp, char* reply) {
	int reply_code = ftp_command(ftp, "PASV", "", reply);

	char* ip_start = strchr(reply, '(') + 1;
	int ip1, ip2, ip3, ip4, gate1, gate2;
	sscanf(ip_start, "%d,%d,%d,%d,%d,%d", &ip1, &ip2, &ip3, &ip4, &gate1, &gate2);

	// determines ip
	char ip[16];
	sprintf(ip, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);

	// determines gate
	int gate = gate1*256 + gate2;

	// opens data socket
	if(ftp_create_socket(&(ftp->data_socket_fd), ip, gate) < 0)
		return -1;
	printf("Data connection was successfuly estabilished\n");

	return 0;
}

int ftp_retr_command(FTP* ftp, char* reply) {
	int reply_code = ftp_command(ftp, "RETR", ftp->url.filename, reply);

	switch(reply_code) {
		case FILE_OK:
			printf("File status ok. About to open data connection\n");
			break;

		case DATA_OPEN:
			printf("Data connection already open. Transfer starting\n");

		default:
			printf("Error\n");
			return -1;
	}

	FILE* file;
	file = fopen(ftp->url.filename, "w");
	if(file == NULL) {
		printf("Unable to create file\n");
		return -1;
	}

	ssize_t bytes_read;
	char buffer[2048];
	while((bytes_read = read(ftp->data_socket_fd, buffer, 2048)) != 0) {
		fwrite(buffer, bytes_read, 1, file);
	}
	fclose(file);

	reply_code = ftp_command(ftp, "RETR", ftp->url.filename, reply);
	switch(reply_code) {
		case FILE_ACTION_OK:
			printf("Download completed\n");
			break;

		case CLOSE_CONNECTION:
			printf("Download completed. Closing data connection\n");
			break;

		default:
			printf("Download failed\n");
			return -1;
	}
}