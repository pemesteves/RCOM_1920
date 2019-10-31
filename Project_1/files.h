#include <sys/types.h>


typedef struct {
    char* file_name;
    off_t file_size;
    int fd;
} applicationLayerFile;

int file_exist(char* file_name);

int get_file_size(applicationLayerFile *file);

/**
 * 
 */
int open_file(applicationLayerFile *file);//char* file_name);

/**
 * 
 */
int read_file(int fd, unsigned char* string, int length);

/**
 * 
 */
int close_file(int fd);

int create_file(applicationLayerFile *file);//char* file_name);

int write_file(int fd, unsigned char* content, unsigned int content_size);