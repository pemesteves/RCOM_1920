#include "app_layer.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int sequence_number = 0;

unsigned char *strrev(unsigned char *str);

int data_packet(int data_length, char * data, unsigned char * packet) {
    packet[0] = DATA;
    packet[1] = sequence_number%256;
    sequence_number++;
    packet[2] = data_length/256;
    packet[3] = data_length%256;

    if(memcpy(&packet[4], data, data_length) == -1)
        return -1;

    return 0;
}

int control_packet(applicationLayerFile *file, char control, char * packet, int *packet_size){
    packet[0] = control; //Control octet: 2 - start packet; 3 - end packet
    if(control == START){
        int file_name_size = strlen(file->file_name);
        packet[1] = 0; // T1 - file size
    
        if(get_file_size(file)){
            printf("Error while getting the file size\n\n");
            return -1;
        }

        off_t v1_file_size = file->file_size; 
        
        int v1_length = 0;
        char* v1 = NULL;
        do{
            v1_length++;
            v1 = realloc(v1, v1_length);
            v1[v1_length-1] = (unsigned char)v1_file_size%256;

            off_t offset = v1_file_size/256;
            if(offset == 0)
                break;
            v1_file_size = offset;
        }while(1);

        v1 = strrev(v1); //Reverse length
        
        packet[2] = v1_length;
        *packet_size += v1_length;
        packet = realloc(packet, *packet_size);
        
        if(memcpy(&packet[3], v1, v1_length) == NULL){
            printf("Error in memcpy\n");
            return -1;
        }
        
        int t2_position = 3 + v1_length;

        packet[t2_position] = 1; //T2 - file name
        packet[t2_position+1] = (unsigned char)file_name_size;
        if(memcpy(&packet[t2_position+2], file->file_name, file_name_size) == NULL){
            printf("Error in memcpy\n");
            return -1;
        }
    }

	return 0;
}

int parse_data_packet(unsigned char *packet, unsigned char **content, unsigned int *content_size) {
    if(packet[1] != sequence_number%256){
        printf("Received data packet with wrong sequence number\n");
        sequence_number = 0;
        return -1;
    }

    sequence_number++;
    *content_size = (packet[2] << 8) + packet[3];
    *content = (unsigned char*)malloc((*content_size)*sizeof(unsigned char));
    if(*content == NULL){
        printf("Error in malloc\n\n");
        return -1;
    }

    for(unsigned int i = 0; i < *content_size; i++){
        (*content)[i] = packet[i+4];
    }

    return 0;
}

int parse_control_packet(unsigned char *packet, unsigned int packet_size, applicationLayerFile *file){//char **file_name, off_t *file_size) {

    for (int i = 1; i < packet_size ; i++){
        if (packet[i] == SIZE) {
            i++;

            unsigned int length = (unsigned int) packet[i];
            file->file_size = 0;
            for (int j = 1; j <= length ; j++)
                file->file_size = ((file->file_size)<<8) + (off_t)packet[i+j];

            i += length;
        }
        else if (packet[i] == NAME) {
            i++;

            unsigned int length = (unsigned int) packet[i];

            file->file_name = (char*)malloc(length + 1);

            if(file->file_name == NULL){
                printf("malloc error\n\n");
                return -1;
            }

            for (int j = 1; j <= length ; j++) {
                (file->file_name)[j-1] = packet[i+j];
            }

            (file->file_name)[length] = '\0';

            i += length;
        }
        else break;
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


