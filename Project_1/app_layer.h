#include <sys/types.h>
#include "files.h"

#define DATA 1
#define START 2
#define END 3

#define SIZE 0
#define NAME 1

/**
 * @brief Function used to create data packets
 * 
 * @param data_length Length of data to send 
 * @param data Data to send
 * @param packet Packet created with the other two parameters. This is an output parameter and it needs to have allocated memory.  
 *
 * @return Returns -1 if there was an error. Otherwise returns 0
 */
int create_data_packet(int data_length, char * data, unsigned char * packet);

/**
 * @brief Function used to create control packets
 * 
 * @param file Struct that contains information about the file 
 * @param control Indicates if it is a start or end control packet
 * @param packet Control packet created with the other two parameters. This is an output parameter and it needs to have allocated memory.  
 * @param packet_size Output parameter that has the packet size.
 *
 * @return Returns -1 if there was an error. Otherwise returns 0
 */
int create_control_packet(applicationLayerFile *file, char control, char * packet, int *packet_size);

/**
 * @brief Function used to parse data packets created by create_data_packet
 * 
 * @param packet Data packet received 
 * @param content Useful content of the packet. Warning: memory allocated inside this function
 * @param content_size Output parameter that has the content size.
 * 
 * @return Returns -1 if there was an error. Otherwise returns 0
 */
int parse_data_packet(unsigned char *packet, unsigned char **content, unsigned int *content_size);

/**
 * @brief Function used to parse control packets created by create_control_packet
 * 
 * @param packet Control packet received 
 * @param packet_size Length of the control packet received
 * @param file Struct that contains information about the file 
 *
 * @return Returns -1 if there was an error. Otherwise returns 0
 */
int parse_control_packet(unsigned char *packet, unsigned int packet_size, applicationLayerFile *file); //char **file_name, off_t *file_size); 