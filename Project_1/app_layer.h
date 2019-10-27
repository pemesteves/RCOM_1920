#define DATA 1
#define START 2
#define END 3

#define SIZE 0
#define NAME 1


int data_packet(int data_lenght, char * data, char * packet);

int control_packet(char * file_name, char control, char * packet);

int read_packet(char * packet, char * file_name, unsigned int * file_size, char * content);
