/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

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

  int fd;
  struct termios oldtio, newtio;

  if((fd = llopen(argv[1], RECEIVER, &oldtio)) < 0){
    printf("\nError in llopen\n");
    return -1;
  }

  char *content;
  applicationLayerFile file;

  bool received_end_packet = false;
  bool received_start_packet = false;

  for(;;){
    unsigned char data[256];
    int data_size = 0;

    if((data_size = llread(fd, &data)) < 0){
      printf("\nError in llread\n");
      return -1;
    }
/*
    printf("\nMessage received: ");
    for(int i = 0; i < data_size; i++) {
      printf("%c", data[i]);
    }
*/

    switch(data[0]) {
      case DATA: // Data packet
      {
        if(!received_start_packet){
          printf("First packet received should be the start control packet\n\n");
          return -1;
        }

        unsigned char *content;
        unsigned int content_size;
        if(parse_data_packet(data, &content, &content_size) < 0){
          printf("Can't parse the data packet\n\n");
          return -1;
        }
        if(write_file(file.fd, content, content_size) < 0){
          printf("Can't write in the new file\n\n");
          return -1;
        }
        free(content);
        break;
      }

      case START: // Start control packet
      {
        if(parse_control_packet(data, data_size, &file) < 0){
          printf("Can't parse the starting control packet\n\n");
          return -1;
        }
        printf("File size: %ld \n\n", file.file_size);
        printf("FILENAME: %s\n", file.file_name);

        if(create_file(&file) < 0){
          printf("Can't open the new file\n\n");
          return -1;
        }
        received_start_packet = true;
        break;
      }

      case END: // End control packet
      {
        if(!received_start_packet){
          printf("First packet received should be the start control packet\n\n");
          return -1;
        }

        applicationLayerFile new_file_data;

        if(parse_control_packet(data, data_size, &new_file_data) < 0){
          printf("Can't parse the starting control packet\n\n");
          return -1;
        }

        if(strcmp(new_file_data.file_name, file.file_name) != 0){
          printf("Received wrong file name\n\n");
          return -1;
        }
        if(new_file_data.file_size != file.file_size){
          printf("NEW FILE SIZE: %ld\n", new_file_data.file_size);
          printf("OLD FILE SIZE: %ld\n", file.file_size);
          printf("Received wrong file size\n\n");
          return -1;
        }
        
        if(get_file_size(&file) < 0){
          printf("Error in the get_file_size function\n\n");
          return -1;
        }
        
        if(file.file_size != new_file_data.file_size){
          printf("EXPECTED FILE SIZE: %ld\n", new_file_data.file_size);
          printf("REAL FILE SIZE: %ld\n", file.file_size);
          printf("File size is different than expected\n\n");
          return -1;
        }
        if(close_file(file.fd) < 0){
          printf("Can't close file\n\n");
          return -1;
        }
        free(new_file_data.file_name);
        received_end_packet = true;
        break;
      }
    }

    if(received_end_packet)
      break;

    printf("\n");
  }

  free(file.file_name);

  if(llclose(fd, &oldtio, RECEIVER)){
		printf("\nllclose error\n");
    return -1;
  }

  return 0;

}
