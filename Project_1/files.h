#include <sys/types.h>

int file_exist(char* file_name);

off_t get_file_size(char* file_name);

/**
 * 
 */
int open_file(char* file_name);

/**
 * 
 */
int read_file(int fd, unsigned char* string, int length);

/**
 * 
 */
int close_file(int fd);


//int create_file(char * file_name, unsigned int file_size, char * content);

int create_file(char* file_name);

int write_file(int fd, unsigned char* content, unsigned int content_size);