#include "prot_app.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

int llopen(char* gate, int flag, struct termios *oldtio)
{
    if (flag != TRANSMITTER && flag != RECEIVER)
        return -1;

    int fd, c, res;
    struct termios newtio;
    char buf[255];
    int i, sum = 0, speed = 0;

    if ((strcmp("/dev/ttyS0", gate) != 0) && (strcmp("/dev/ttyS1", gate) != 0)))
    {
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
        return -1;
    }

    /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
    */
    fd = open(gate, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror(gate);
        return -1;
    }

    if (tcgetattr(fd, oldtio) == -1)
    { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    if(flag == TRANSMITTER){
        newtio.c_cc[VTIME] = 3; /* inter-character timer unused */
        newtio.c_cc[VMIN] = 0;  /* blocking read until 0 chars received */
    }
    else{
        newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
        newtio.c_cc[VMIN] = 1;  /* blocking read until 1 char received */
    }

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }
    printf("New termios structure set\n");

    return -1;
}

int llwrite(int fd, char *buffer, int length)
{
    return 0;
}

int llread(int fd, char *buffer)
{
    return 0;
}

int llclose(int fd)
{
    return 0;
}