#include <termios.h>

#define TRANSMITTER 0
#define RECEIVER 1

#define SUPERVISION_PLOT_SIZE 5

/* Main Functions */

int llopen(char* porta, int flag, struct termios *oldtio);

int llwrite(int fd, char * buffer, int length);

int llread(int fd, char * buffer);

int llclose(int fd, struct termios *oldtio);


/* Auxiliary Functions */

unsigned char* create_supervision_plot(char control_field);
int receive_supervision_plot(int fd);
int send_supervision_plot(int fd, char control_field);

void create_information_plot(char control_field, char *data, int length, unsigned char* plot);
int receive_information_plot(int fd, unsigned char *received_plot, int *received_plot_length);

int send_plot(int fd, unsigned char* plot);


/* Calculation functions */

unsigned char* retrieve_data(unsigned char *information_plot, int plot_length, int *data_length);
unsigned char retrieve_bcc2(unsigned char *information_plot, int plot_length);
unsigned char calculate_bcc2(unsigned char *data, int data_length);


/* Print */

void print_string(unsigned char *string, int length);
