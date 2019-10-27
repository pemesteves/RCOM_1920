#include "app_layer.h"
#include "files.h"


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int sequence_number = 0;

int data_packet(int data_lenght, char * data, char * packet) {
    packet[0] = DATA;
    packet[1] = sequence_number;
    sequence_number++;
    packet[2] = data_lenght/256;
    packet[3] = data_lenght%256;

    if(memcpy(&packet[4], data, data_lenght) == -1)
        return -1;

    return 0;
}

int control_packet(char * file_name, char control, char * packet){
	packet = malloc(8 + sizeof(file_name)/sizeof(file_name[0]));
    packet[0] = control;
	packet[1] = 0;
    packet[2] = sizeof(int);
    packet[3] = (char)(get_file_size(file_name) % 0x0010);
    packet[4] = (char)(get_file_size(file_name) / 0x0010 % 0x0010);
    packet[5] = (char)(get_file_size(file_name) / 0x0100 % 0x0010);
    packet[6] = (char)(get_file_size(file_name) / 0x1000);
	packet[7] = 1;
	packet[8] = (char)(sizeof(file_name)/sizeof(file_name[0]));
    strcpy(packet + 9, file_name);
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





