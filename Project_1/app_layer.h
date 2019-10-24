/**
 * 
 */
int open_file(char* file_name);

/**
 * 
 */
int read_file(int fd, unsigned char* string);

/**
 * 
 */
int close_file(int fd);


int data_packet(char * string, char * packet);

int control_packet(char * packet, char * file_name, int control);
