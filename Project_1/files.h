#include <sys/types.h>

/**
 * @brief Struct type to specify the file information
 */
typedef struct {
    char* file_name; /**< @brief file name */
    off_t file_size; /**< @brief file size */
    int fd;          /**< @brief file descriptor */
} applicationLayerFile;

/**
 * @brief Function that verifies if a file exists
 * 
 * @param file_name File name that will be verified
 * 
 * @return Returns -1 if there was an error. Otherwise returns 0
 */
int file_exist(char* file_name);

/**
 * @brief Function that gets the file size
 * 
 * @param file Struct that has already the file name and will also have the file size
 * 
 * @return Returns -1 if there was an error. Otherwise returns 0
 */
int get_file_size(applicationLayerFile *file);

/**
 * @brief Function that opens a file for reading
 * 
 * @param file Struct that has already the file name and will also have the file descriptor
 * 
 * @return Returns -1 if there was an error. Otherwise returns 0
 */
int open_file(applicationLayerFile *file);

/**
 * @brief Function that reads length bytes from a file
 * 
 * @param fd Descriptor of the file to be read
 * @param string String read from the file
 * @param length Maximum number of characters to read
 * 
 * @return Returns the number of characters read from the file. Returns -1 if there was an error.
 */
int read_file(int fd, unsigned char* string, int length);

/**
 * @brief Function that closes a file
 * 
 * @param fd Descriptor of the file to be closed
 * 
 * @return Returns -1 if there was an error. Otherwise returns 0
 */
int close_file(int fd);

/**
 * @brief Function that opens a file for writing 
 * 
 * @param file Struct that has already the file name and will also have the file descriptor
 * 
 * @return Returns -1 if there was an error. Otherwise returns 0
 */
int create_file(applicationLayerFile *file);

/**
 * @brief Function that writes content_size bytes in a file
 * 
 * @param fd Descriptor of the file 
 * @param content Content to write on the file
 * @param content_size Size of the content to write
 * 
 * @return Returns -1 if there was an error. Otherwise returns 0
 */
int write_file(int fd, unsigned char* content, unsigned int content_size);