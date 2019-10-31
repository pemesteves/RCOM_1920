#include <sys/types.h>
#include "files.h"

#define DATA 1
#define START 2
#define END 3

#define SIZE 0
#define NAME 1


int data_packet(int data_length, char * data, unsigned char * packet);

int control_packet(applicationLayerFile *file, char control, char * packet, int *packet_size);

int parse_data_packet(unsigned char *packet, unsigned char **content, unsigned int *content_size);

int parse_control_packet(unsigned char *packet, unsigned int packet_size, applicationLayerFile *file); //char **file_name, off_t *file_size); 