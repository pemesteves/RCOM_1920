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

unsigned char* create_information_plot(char control_field, char *data, int length);

int send_plot(int fd, unsigned char* plot);

