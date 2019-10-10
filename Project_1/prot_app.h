
#define TRANSMITTER 0
#define RECEIVER 1

int llopen(int porta, int flag);

int llwrite(int fd, char * buffer, int length);

int llread(int fd, char * buffer);

int llclose(int fd);