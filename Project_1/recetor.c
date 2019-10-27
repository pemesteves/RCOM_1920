/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "link_layer.h"
#include "app_layer.h"

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
  int current = 0;
  int max = 4;

  int fd, res;
  struct termios oldtio,newtio;
  char buf[5];
  char * file_name, * file_size, *content;

  if ( (argc < 2) ||
      ((strcmp("/dev/ttyS0", argv[1])!=0) &&
       (strcmp("/dev/ttyS1", argv[1])!=0) &&
       (strcmp("/dev/ttyS2", argv[1])!=0))) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }

  if((fd = llopen(argv[1], RECEIVER, &oldtio)) < 0){
    printf("\nError in llopen\n");
    return -1;
  }

  for(;;){
    char data[128];
    int data_size = 0;
    
    if((data_size = llread(fd, &data)) < 0){
      printf("\nError in llread\n");
      return -1;
    }

    printf("\nMessage received: ");
    for(int i = 0; i < data_size; i++) {
      printf("%c", data[i]);
    }

    //read_packet(data, file_name, file_size, content);
    //printf("\nRead packet\n");
    //printf("File size: %d\n", file_size);
    //printf("File name: %d\n", file_name);
    //printf("Content: %d\n", content);

    printf("\n");
  }

  sleep(2);

  if(llclose(fd,& oldtio)){
		printf("\nllclose error\n");
    return -1;
  }

  //create_file(file_name, file_size, content);

  return 0;

}