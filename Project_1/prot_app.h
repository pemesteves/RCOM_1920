#define TRANSMITTER 0
#define RECEIVER 1

#include <termios.h>

int llopen(int porta, int flag, struct termios *oldtio);

int llwrite(int fd, char * buffer, int length);

int llread(int fd, char * buffer);

int llclose(int fd);