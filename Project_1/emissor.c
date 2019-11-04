/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "link_layer.h"
#include "app_layer.h"

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		printf("Emissor should have at least 3 parameters: ./emissor serial_port_path file_path\n\n");
		return -1;
	}

	int serial_fd, c, res;

	int num_bytes_read;

	//File Information
	applicationLayerFile fileInfo;

	fileInfo.file_name = argv[2];

	//Checks if the file received by argument exists
	if (file_exist(fileInfo.file_name))
	{
		printf("File %s doesn't exist!!!\n\n", fileInfo.file_name);
		return -1;
	}
	else
	{
		printf("File %s exist\n\n", fileInfo.file_name);
	}

	//Opens the communication protocol
	struct termios oldtio;
	if ((serial_fd = llopen(argv[1], TRANSMITTER, &oldtio)) < 0)
	{
		printf("llopen error\n");
		return -1;
	}

	//Opens the received file  
	if (open_file(&fileInfo) < 0)
	{
		printf("Can't open %s!\n\n", fileInfo.file_name);
		return -1;
	}
	printf("Opened file %s\n\n", fileInfo.file_name);

	int control_packet_length = 5 * sizeof(unsigned char) + strlen(fileInfo.file_name);
	int first_control_packet_length = control_packet_length;

	unsigned char *ctrl_packet = (unsigned char *)malloc(control_packet_length);
	
	//Creates the start control packet
	if (create_control_packet(&fileInfo, START, ctrl_packet, &control_packet_length) < 0)
	{
		printf("Error in start create_control_packet\n\n");
		return -1;
	}

	//Prints to check if the control packet is right
	printf("START CONTROL PACKET: ");
	for (int i = 0; i < control_packet_length; i++)
	{
		printf("%x ", ctrl_packet[i]);
	}
	printf("\n");

	//Sends the start control packet to the receiver
	printf("Sending start control packet...\n");
	if (llwrite(serial_fd, ctrl_packet, control_packet_length) < 0)
	{
		printf("llwrite error\n");
		return -1;
	}
	else
	{
		printf("Sent start control packet\n\n");
	}

	int data_length = 256;
	int data_packet_length = 0;
	unsigned char *data = (unsigned char *)malloc(data_length * sizeof(char) + 1);

	unsigned char *data_pkt;

	//Loop where the data will be read from the file and sent to the receiver
	for (;;)
	{
		memset(data, '\0', data_length);

		//Reads data from the file
		num_bytes_read = read_file(fileInfo.fd, data, data_length);
		if (num_bytes_read < 0)
		{
			printf("Error: read file\n\n");
			return -1;
		}
		else if (num_bytes_read == 0)
		{
			printf("End of file\n\n");
			break;
		}
		printf("Read %i bytes from file %s\n\n", num_bytes_read, fileInfo.file_name);

		data_packet_length = 4 + num_bytes_read;
		data_pkt = (unsigned char *)malloc(data_packet_length);
		
		//Creates the data packet 
		if (create_data_packet(num_bytes_read, data, data_pkt))
		{
			printf("Error while creating the data packet\n\n");
			return -1;
		}

		printf("Sending message...\n");

		//Sends the data packet to the receiver
		if (llwrite(serial_fd, data_pkt, data_packet_length) < 0)
		{
			printf("llwrite error\n");
			return -1;
		}
		else
		{
			printf("Sent data packet\n\n");
		}

		free(data_pkt);
	}
	free(data);

	//Creates the end control packet
	if (create_control_packet(&fileInfo, END, ctrl_packet, &control_packet_length) < 0)
	{
		printf("Error in end create_control_packet\n\n");
		return -1;
	}
	printf("END CONTROL PACKET: ");
	for (int i = 0; i < control_packet_length; i++)
	{
		printf("%x ", ctrl_packet[i]);
	}
	printf("\n");

	printf("Sending end control packet...\n");
	
	//Sends the end control packet
	if (llwrite(serial_fd, ctrl_packet, control_packet_length) < 0)
	{
		printf("llwrite error\n");
		return -1;
	}
	else
	{
		printf("Sent end control packet\n\n");
	}
	free(ctrl_packet);

	//Closes the received file  
	if (close_file(fileInfo.fd) < 0)
	{
		printf("Can't close %s!\n\n", fileInfo.file_name);
		return -1;
	}

	//Closes the communication protocol
	if (llclose(serial_fd, &oldtio, TRANSMITTER))
	{
		printf("llclose error\n");
		return -1;
	}

	return 0;
}
