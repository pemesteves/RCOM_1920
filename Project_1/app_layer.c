#include "app_layer.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int sequence_number = 0;

/**
 * @brief Function that reverses a string
 * 
 * @param str String 
 * 
 * @return Returns the reversed string
 */
unsigned char *strrev(unsigned char *str, int length);


int create_data_packet(int data_length, char * data, unsigned char * packet) {
    packet[0] = DATA; //Indicates that it's a data packet
    packet[1] = sequence_number%256; //Sequence number: 0..255
    sequence_number++;

    //Number that represents the data length could have 2 bytes
    packet[2] = data_length/256; 
    packet[3] = data_length%256;

    //Copying the data to the packet
    if(memcpy(&packet[4], data, data_length) == NULL)
        return -1;

    return 0;
}

int create_control_packet(applicationLayerFile *file, char control, char * packet, int *packet_size){
    packet[0] = control; //Control octet: 2 - start packet; 3 - end packet
    
    /*
        If we want to create the start control packet, we'll put the file size TLV and the file name TLV (TLV = Type, Length, Value)
        If we want to create the end control packet, we'll do nothing, because the control packet is the same 
            It's assured that this function receives the control packet that was created in the beginning 
    */
    if(control == START){  
        int file_name_size = strlen(file->file_name);
        packet[1] = 0; // T1 - file size
    
        if(get_file_size(file)){ //Getting the file size
            printf("Error while getting the file size\n\n");
            return -1;
        }

        //Creating the L and V field of the file size -> off_t variable is a 64 bit integer
        off_t v1_file_size = file->file_size; 

        int v1_length = 0;
        unsigned char* v1 = NULL;
        do{
            v1_length++;
            v1 = realloc(v1, v1_length); //Reallocating memory to v field of the file size
            v1[v1_length-1] = (unsigned char)v1_file_size%256; //We'll have the most significant bits of the length on the right
            
            off_t offset = v1_file_size/256;
            if(offset == 0)
                break;
            v1_file_size = offset;
        }while(1);

        strrev(v1, v1_length); //Reverse length so that the most significant bits are on the left like it's supposed to be

        //Copying the file size TLV to the packet
        packet[2] = v1_length; 
        *packet_size += v1_length; //Packet size will increase v1_length
        packet = realloc(packet, *packet_size);
        
        //Copying the value of file size to the packet
        if(memcpy(&packet[3], v1, v1_length) == NULL){
            printf("Error in memcpy\n");
            return -1;
        }
        
        //The type of the file name TLV will be on the position 3+v1_length
        int t2_position = 3 + v1_length;

        //Copying the file name TLV to the packet
        packet[t2_position] = 1; //T2 - file name
        packet[t2_position+1] = (unsigned char)file_name_size;

        //Copying the file name to the packet
        if(memcpy(&packet[t2_position+2], file->file_name, file_name_size) == NULL){
            printf("Error in memcpy\n");
            return -1;
        }
    }

	return 0;
}

int parse_data_packet(unsigned char *packet, unsigned char **content, unsigned int *content_size) {
    if(packet[1] != sequence_number%256){ //Checks if the received packet is the correct packet
        printf("Received data packet with wrong sequence number\n");
        sequence_number = 0;
        return -1;
    }

    sequence_number++; 

    //Content size will be on the third and fourth byte of the packet 
    *content_size = (packet[2] << 8) + packet[3];

    //Allocating memory to the content based on the received content size
    *content = (unsigned char*)malloc((*content_size)*sizeof(unsigned char));
    if(*content == NULL){
        printf("Error in malloc\n\n");
        return -1;
    }

    //Parses the content from the packet
    for(unsigned int i = 0; i < *content_size; i++){
        (*content)[i] = packet[i+4];
    }

    return 0;
}

int parse_control_packet(unsigned char *packet, unsigned int packet_size, applicationLayerFile *file){
       
    //First byte of the packet is checked before this function has been called
   
    for (int i = 1; i < packet_size ; i++){
        if (packet[i] == SIZE) { //Checks if the next TLV is the file size
            i++;

            unsigned int length = (unsigned int) packet[i];
            file->file_size = 0;

            //Parsing the file size from the packet
            for (int j = 1; j <= length ; j++)
                file->file_size = ((file->file_size)<<8) + (off_t)packet[i+j];

            i += length;
        }
        else if (packet[i] == NAME) { //Checks if the next TLV is the file name
            i++;

            unsigned int length = (unsigned int) packet[i];

            file->file_name = (char*)malloc(length + 1);

            if(file->file_name == NULL){
                printf("malloc error\n\n");
                return -1;
            }

            //Parsing the file name from the packet
            for (int j = 1; j <= length ; j++) {
                (file->file_name)[j-1] = packet[i+j];
            }

            //The last character has to be the NULL character because file->file_name is a C string
            (file->file_name)[length] = '\0';

            i += length;
        }
        else break;
    }

    return 0;
}

unsigned char *strrev(unsigned char *str, int length)
{
      char *p1, *p2;

      if (! str || ! *str)
            return str;
      for (p1 = str, p2 = str + length - 1; p2 > p1; ++p1, --p2)
      {
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
      }
      return str;
}
