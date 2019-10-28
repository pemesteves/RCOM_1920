#include "app_layer.h"
#include "files.h"


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int sequence_number = 0;

unsigned char *strrev(unsigned char *str);

int data_packet(int data_length, char * data, char * packet) {
    packet[0] = DATA;
    packet[1] = sequence_number;
    sequence_number++;
    packet[2] = data_length/256;
    packet[3] = data_length%256;

    if(memcpy(&packet[4], data, data_length) == -1)
        return -1;

    return 0;
}

int control_packet(char * file_name, char control, char * packet, int packet_size){
	int file_name_size = sizeof(file_name)/sizeof(file_name[0]);

    if(packet == NULL){
        printf("Error in malloc\n\n");
        return -1;
    }
    packet[0] = control; //Control octet: 2 - start packet; 3 - end packet
	
    packet[1] = 0; // T1 - file size

    off_t file_size =  get_file_size(file_name);

    int v1_length = 0;
    char* v1 = NULL;
    do{
        v1_length++;
        v1 = realloc(v1, v1_length);
        v1[v1_length-1] = (unsigned char)file_size%256;

        off_t offset = file_size/256;
        if(offset == 0)
            break;
        file_size = offset;
    }while(1);

    v1 = strrev(v1); //Reverse length
    
    packet[2] = v1_length;
    packet_size += v1_length;
    packet = realloc(packet, packet_size);

    if(memcpy(&packet[3], v1, v1_length) == NULL){
        printf("Error in memcpy\n");
        return -1;
    }
    
    int t2_position = 3 + v1_length;

    packet[t2_position] = 1; //T2 - file name
	packet[t2_position+1] = (unsigned char)file_name_size;
	if(memcpy(&packet[t2_position+2], file_name, file_name_size) == NULL){
        printf("Error in memcpy\n");
        return -1;
    }
    
	return 0;
}

int read_packet(char * packet, char * file_name, unsigned int * file_size, char * content){
    if (packet[0] == START) {
        char * name;
        for (int i = 1; i < sizeof(packet) ; i++){
            if (packet[i] == SIZE) {
                i++;
                unsigned int length = packet[i];
                *file_size = 0;
                for (int j = 1; j <= length ; j++) {
                    *file_size += packet[i+j] << ((j-1)*8);
                }
                i += length;
            }
            else if (packet[i] == NAME) {
                i++;
                unsigned int length = packet[i];
                file_name = malloc(length);
                for (int j = 1; j <= length ; j++) {
                    file_name[j-1] = packet[i+j];
                }
                i += length;
            }
            else break;
        }
    }
    else if (packet[0] == DATA) {
        if (packet[1] != sequence_number) {
            printf("Received data packet with wrong sequence number");
            sequence_number = 0;
            return -1;
        }
        unsigned int length = packet[2] + (packet[3] << 8);
        content = malloc(length);
        for (int i = 0; i < length ; i++) {
            content = packet[4 + i];
        }
    }
    else {
        printf("Received packet with invalid control number");
        sequence_number = 0;
        return -1;
    }

    return 0;

}

unsigned char *strrev(unsigned char *str)
{
      char *p1, *p2;

      if (! str || ! *str)
            return str;
      for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
      {
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
      }
      return str;
}


