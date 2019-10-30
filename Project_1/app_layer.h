#include <sys/types.h>

#define DATA 1
#define START 2
#define END 3

#define SIZE 0
#define NAME 1


int data_packet(int data_length, char * data, unsigned char * packet);

int control_packet(char * file_name, char control, char * packet, int *packet_size, off_t *file_size);

int parse_data_packet(char *packet, unsigned char *content, unsigned int *content_size);

int parse_control_packet(char *packet, unsigned int packet_size, char *file_name, off_t *file_size);