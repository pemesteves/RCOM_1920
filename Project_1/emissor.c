/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "link_layer.h"
#include "app_layer.h"
#include "files.h"

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1


int main(int argc, char** argv)
{
	int serial_fd,c, res;

	//File variables
	int fd, num_bytes_read;

	char * file_name = "./pinguim.gif";
	
	struct termios oldtio;

	if((serial_fd = llopen(argv[1], TRANSMITTER, &oldtio)) < 0){
		printf("llopen error\n");
		return -1;
	}

	if(file_exist("./pinguim.gif")){
		printf("File %s doesn't exist!!!\n\n", "./pinguim.gif");
		return -1;
	}
	else{
		printf("File %s exist\n\n", "./pinguim.gif");
	}

	if((fd = open_file("./pinguim.gif")) < 0){
		printf("Can't open %s!\n\n", "./pinguim.gif");
		return -1;
	}
	printf("Opened file %s\n\n", "./pinguim.gif");

	int data_length = 100;
	unsigned char *data = (unsigned char*)malloc(data_length*sizeof(char)+1);
	
	unsigned char *data_pkt;
	
	for(;;){
		memset(data, '\0', data_length);
		num_bytes_read = read_file(fd, data, data_length);

		if(num_bytes_read < 0){
			printf("Error: read file\n\n");
			return -1;
		} else if(num_bytes_read == 0){
			printf("End of file\n\n");
			break;
		}
		printf("Read %i bytes from file %s\n\n", num_bytes_read, "./pinguim.gif");

		printf("Sending message...\n");

   		data_pkt = (unsigned char*)malloc(num_bytes_read+4*sizeof(char));
		if(data_packet(data, data_pkt)){
			printf("Error while creating the data packet\n\n");
			return -1;
		}

		if(llwrite(serial_fd, data_pkt, 4+num_bytes_read) < 0){
			printf("llwrite error\n");
			return -1;
		}
		else {
			printf("Sent data packet\n");
		}

		free(data_pkt);
	}
	

	free(data);

	//sleep(2);

	if(close_file(fd) < 0){
		printf("Can't close %s!\n\n", "./pinguim.gif");
		return -1;
	}

	if(llclose(serial_fd,& oldtio)){
		printf("llclose error\n");
		return -1;
	}

  return 0;
}
