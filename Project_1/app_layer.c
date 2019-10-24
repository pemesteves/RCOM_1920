#include "app_layer.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>

#define START 2
#define END 3

int sequence_number = 0;

int data_packet(char * string, char * packet) {
    packet = malloc(4 + sizeof(string)/sizeof(string[0]));
    packet[0] = 1;
    packet[1] = sequence_number;
    sequence_number++;
    packet[2] = (sizeof(string)/sizeof(string[0]))/256;
    packet[3] = (sizeof(string)/sizeof(string[0]))%256;
    memcpy(packet + 4, string, sizeof(string)/sizeof(string[0]));
    return 0;

}

int control_packet(char * packet, char * file_name, int control){
    packet[0] = control;
    packet[1] = get_file_size(file_name);
    memcpy(packet + 2, file_name, sizeof(file_name)/sizeof(file_name[0]));
}
