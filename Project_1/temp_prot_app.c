#include "prot_app.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x7e
#define A 0x03
#define C_SET 0x03

int alarm_flag = 1, counter = 1;

/*
 * Signal Handler for SIGALRM
 */
void atende(int signal);

int llopen(char *gate, int flag, struct termios *oldtio)
{

    if (flag != TRANSMITTER && flag != RECEIVER)
        return -1;

    __sighandler_t sigalrm_handler;

    if(flag == TRANSMITTER){
        sigalrm_handler = signal(SIGALRM, atende);  // instala  rotina que atende interrupcao
    }

    int fd;
    struct termios newtio;
    char buf[255];

    if ((strcmp("/dev/ttyS0", gate) != 0) && (strcmp("/dev/ttyS1", gate) != 0))
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
        return -1;
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    if (flag == TRANSMITTER)
    {
        newtio.c_cc[VTIME] = 3; /* inter-character timer unused */
        newtio.c_cc[VMIN] = 0;  /* blocking read until 0 chars received */
    }
    else
    {
        newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
        newtio.c_cc[VMIN] = 1;  /* blocking read until 1 char received */
    }

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        return -1;
    }
    printf("New termios structure set\n");

    int state = 0;
    if (flag == TRANSMITTER)
    {
        unsigned char SET[5]; //[FLAG, A, C_SET, BCC, FLAG]

        while (counter < 4 && state < 5)
        {
            printf("Transmission number %d\n", counter);
            if (alarm_flag)
            {
                alarm(newtio.c_cc[VTIME]);
                alarm_flag = 0;
            }

            SET[0] = FLAG;
            SET[1] = A;
            SET[2] = C_SET;
            SET[3] = SET[1] ^ SET[2]; //BCC
            SET[4] = FLAG;

            if (write(fd, SET, 5) < 0)
            {
                perror("write");
                exit(-1);
            }

            memset(SET, '\0', sizeof(SET));

            state = 0;
            while (state != 5) /* loop for input */
            {
                if (alarm_flag == 1)
                {
                    printf("Receiving error at retransmission number %d\n", counter - 1);
                    break;
                }
                read(fd, &SET[state], 1); /* returns after 5 chars have been input */
                switch (state)
                {
                case 0:
                    if (SET[state] == FLAG)
                        state = 1;
                    break;
                case 1:
                    if (SET[state] == A)
                        state = 2;
                    else if (SET[state] == FLAG)
                        state = 1;
                    else
                        state = 0;
                    break;
                case 2:
                    if (SET[state] == C_SET)
                        state = 3;
                    else if (SET[state] == FLAG)
                        state = 1;
                    else
                        state = 0;
                    break;
                case 3:
                    if (SET[state] == C_SET ^ A)
                        state = 4;
                    else
                        state = 0;
                    break;
                case 4:
                    if (SET[state] == FLAG)
                        state = 5;
                    else
                        state = 0;
                    break;
                default:
                    break;
                }
            }
        }
    }
    else
    {
        while (state != 5)
        {                             /* loop for input */
            read(fd, &buf[state], 1); /* returns after 5 chars have been input */
            printf("STATE: %d\n", state);
            switch (state)
            {
            case 0:
                if (buf[state] == FLAG)
                    state = 1;
                break;
            case 1:
                if (buf[state] == A)
                    state = 2;
                else if (buf[state] == FLAG)
                    state = 1;
                else
                    state = 0;
                break;
            case 2:
                if (buf[state] == C_SET)
                    state = 3;
                else if (buf[state] == FLAG)
                    state = 1;
                else
                    state = 0;
                break;
            case 3:
                if (buf[state] == C_SET ^ A)
                    state = 4;
                else
                    state = 0;
                break;
            case 4:
                if (buf[state] == FLAG)
                    state = 5;
                else
                    state = 0;
                break;
            default:
                break;
            }
        }

        int res = write(fd, buf, 5);
        printf("%d bytes written\n", res);

        sleep(1);
    }

    if(flag == TRANSMITTER){
        (void) signal(SIGALRM, sigalrm_handler);
    }

    return fd;
}

int llwrite(int fd, char *buffer, int length)
{
    return write(fd, buffer, length);
}

int llread(int fd, char *buffer)
{
    int flag_counter = 0, char_counter = 0;

    while (flag_counter < 2)
    {
        if (read(fd, &buffer[char_counter], 1) == -1)
        {
            perror("read");
            return 1;
        }

        if (buffer[char_counter] == FLAG)
        {
            flag_counter++;
        }

        char_counter++;
    }

    return char_counter;
}

int llclose(int fd, struct termios *oldtio)
{
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        return 1;
    }

    printf("New termios structure set\n");

    close(fd);
}

void atende(int signal)
{
    printf("alarme # %d\n", counter);
    alarm_flag = 1;
    counter++;
}
