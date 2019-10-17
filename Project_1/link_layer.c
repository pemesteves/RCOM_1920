#include "link_layer.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG    0x7e
#define A       0x03
#define C_SET   0x03
#define C_UA    0x07
#define C_RR    0x05

int alarm_flag = 1, counter = 1;

/*
 * Signal Handler for SIGALRM
 */
void atende(int signal);

int llopen(char *gate, int flag, struct termios *oldtio)
{
    if (flag != TRANSMITTER && flag != RECEIVER)
        return -1;

	void* sigalrm_handler;

    if(flag == TRANSMITTER){
        sigalrm_handler = signal(SIGALRM, atende);  // instala  rotina que atende interrupcao
    }

    int fd;
    struct termios newtio;
    char buf[255];

    if ((strcmp("/dev/ttyS0", gate) != 0) && (strcmp("/dev/ttyS1", gate) != 0) && (strcmp("/dev/ttyS2", gate) != 0))    //S2 para teste em VB
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

    printf("New termios structure set at llopen\n");

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

            int size_written;

            if ((size_written = write(fd, SET, 5)) < 0)
            {
                perror("write");
                exit(-1);
            }

            printf("%d bytes written: ", size_written);
            printf("%x %x %x %x %x\n", SET[0], SET[1], SET[2], SET[3], SET[4]);

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
                    if (SET[state] == C_UA)
                        state = 3;
                    else if (SET[state] == FLAG)
                        state = 1;
                    else
                        state = 0;
                    break;
                case 3:
                    if (SET[state] == C_UA ^ A)
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
	   
	if(counter >= 4){
            write(stderr, "Can't open protocol\n", 21);
	    return -1;
        }
    }
    else
    {
        while (state != 5)
        {                             /* loop for input */
            read(fd, &buf[state], 1); /* returns after 5 chars have been input */
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

        unsigned char UA[5];

        UA[0] = FLAG;
        UA[1] = A;
        UA[2] = C_UA;
        UA[3] = UA[1] ^ UA[2]; //BCC
        UA[4] = FLAG;

		
  		int size_written = write(fd, UA, 5);
  		printf("%d bytes written: ", size_written);
        printf("%x %x %x %x %x\n", UA[0], UA[1], UA[2], UA[3], UA[4]);
    }

    if(flag == TRANSMITTER){
          alarm(0);
        (void) signal(SIGALRM, sigalrm_handler);
    }

    return fd;
}

int llwrite(int fd, char *buffer, int length)
{if(length <= 0){
      write(stderr, "Length must be positive!\n", 26);
      return -1;
    }

    unsigned char* I = malloc(6*sizeof(char)+length*sizeof(char)); //[FLAG, A, C_SET, BCC1, [DADOS], BCC2, FLAG]

  /*  while (counter < 4 && state < 5)
    {
        printf("Transmission number %d\n", counter);
        if (alarm_flag)
        {
            alarm(newtio.c_cc[VTIME]);
            alarm_flag = 0;
        }
*/
        I[0] = FLAG;
        I[1] = A;
        I[2] = 0x00;//C_SET;
        I[3] = I[1] ^ I[2]; //BCC1

        char BCC2 = 0;
        int i_index = 4;
        for(int i = 0; i < length; i++){ //I think this can be changed -> strcpy
            I[i_index] = buffer[i];
            BCC2 ^= buffer[i];
            i_index++;
        }
        I[i_index] = BCC2;
        i_index++;
        I[i_index] = FLAG;

        int size_written;

        if ((size_written = write(fd, I, length+6)) < 0)
        {
            perror("write");
            return -1;
        }

        printf("%d bytes written. ", size_written);

        for(int i = 0; i < length+6; i++){
            printf("%c ", I[i]);
        }

        free(I);

        unsigned char RR[5];

        int state = 0;

        while (state != 5) /* loop for input */
        {
            if (alarm_flag == 1)
            {
                printf("Receiving error at retransmission number %d\n", counter - 1);
                break;
            }
            read(fd, &RR[state], 1); /* returns after 5 chars have been input */
            switch (state)
            {
            case 0:
                if (RR[state] == FLAG)
                    state = 1;
                break;
            case 1:
                if (RR[state] == A)
                    state = 2;
                else if (RR[state] == FLAG)
                    state = 1;
                else
                    state = 0;
                break;
            case 2:
                if (RR[state] == C_RR)
                    state = 3;
                else if (RR[state] == FLAG)
                    state = 1;
                else
                    state = 0;
                break;
            case 3:
                if (RR[state] == C_RR ^ A)
                    state = 4;
                else
                    state = 0;
                break;
            case 4:
                if (RR[state] == FLAG)
                    state = 5;
                else
                    state = 0;
                break;
            default:
                break;
            }
        }

    return size_written;
}

int llread(int fd, char *buffer)
{
    printf("\nStarting llread\n");

    unsigned char data[128];
    unsigned char buffer_aux[128];

    int state = 0;
    int count = 0;
    int save_state = 0;
    while (state != -1)
    {                             /* loop for input */
        int num = read(fd, &buffer_aux[state], 1); /* returns after 5 chars have been input */

        //printf("buffer: %x\nstate: %d\n\n", buffer_aux[state], state);
        switch (state)
        {
        case 0:
            if (buffer_aux[state] == FLAG) {
                state = 1;
            }
            break;
        case 1:
            if (buffer_aux[state] == A)
                state = 2;
            else if (buffer_aux[state] == FLAG)
                state = 1;
            else
                state = 0;
            break;
        case 2:

            if (buffer_aux[state] == 0x00) { //temporary
                state = 3;
            }
            else if (buffer_aux[state] == FLAG)
            {
                state = 1;
            }
            else
            {
                state = 0;
            }
            break;
        case 3:
            if (buffer_aux[state] == A ^ 0x00)  {//temporary
                state = 4;
            }
            else
            {
                state = 0;
            }
            break;

        default:
            if(buffer_aux[state] == FLAG){
                state = -1;
                break;
            }                    
            data[count] = buffer_aux[state];

            state++;
            count++;

            save_state = state;

            break;
        }
    }

    printf("Received 'plot I'\n");

    unsigned char BCC2 = 0;

    for(unsigned int i = 4; i < save_state - 1; i++) {
        BCC2 ^= buffer_aux[i];
    }

    //printf("\nBCC2: %x\n", BCC2);
    //printf("Save state: %d\n", save_state);

    if (buffer_aux[save_state - 1] != BCC2)
        return -1;

    data[save_state -1] = '/0';
    strcpy(buffer, data);



    unsigned char RR[5];

    RR[0] = FLAG;
    RR[1] = A;
    RR[2] = C_RR;   //NEED TO CHANGE -> C_RR1, C_RR0
    RR[3] = RR[1] ^ RR[2]; //BCC
    RR[4] = FLAG;

    
    int size_written = write(fd, RR, 5);
    printf("%d bytes written: ", size_written);
    printf("%x %x %x %x %x\n", RR[0], RR[1], RR[2], RR[3], RR[4]);

}

int llclose(int fd, struct termios *oldtio)
{
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        return 1;
    }

    printf("\nOld termios structure set at llclose\n");

    close(fd);
}

void atende(int signal)
{
    printf("alarme # %d\n", counter);
    alarm_flag = 1;
    counter++;
}
