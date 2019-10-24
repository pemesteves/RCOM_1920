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

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1


int main(int argc, char** argv)
{
	int fd,c, res;
	char buf[255];

	if ( (argc < 2) ||
	   ((strcmp("/dev/ttyS0", argv[1])!=0) &&
		(strcmp("/dev/ttyS1", argv[1])!=0) &&
		(strcmp("/dev/ttyS2", argv[1])!=0))) {
	printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
	exit(1);
	}

	struct termios oldtio;

	if((fd = llopen(argv[1], TRANSMITTER, &oldtio)) < 0){
		printf("llopen error\n");
		return -1;
	}

	for(int i = 0; i < 3; i++){
		unsigned char data[8];
		sprintf(data, "Hello_%d", i);
		printf("Message\n");

		if(llwrite(fd, data, 5) < 0){
			printf("llwrite error\n");
			return -1;
		}
	}

	//sleep(2);

	if(llclose(fd,& oldtio)){
		printf("llclose error\n");
		return -1;
	}

  return 0;
}
