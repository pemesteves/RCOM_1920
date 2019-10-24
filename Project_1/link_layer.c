#include "link_layer.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define MAX_REJECTS 3
#define BIT(n) 1 << n

#define FLAG        0x7e
#define ESCAPE      0x7D
#define A           0x03
#define C_SET       0x03
#define C_UA        0x07
#define C_RR        0x05
#define C_SEND_0    0x00
#define C_SEND_1    0x40
#define C_RR_0      0x05
#define C_RR_1      0x85
#define C_REJ_0     0x01
#define C_REJ_1     0x81

int alarm_flag = 1, counter = 1;

unsigned char sender_field = C_SEND_0;
unsigned char receiver_ready_field = C_RR_1;
unsigned char receiver_reject_field = C_REJ_1;

unsigned int timeout;

int information_plot_counter = 1;

/**
 * Signal Handler for SIGALRM
 */
void atende(int signal);

/**
 * Update sender and receiver numbers
 */
void update_transm_nums();

/**
 * Byte Stuffing
 */
void byte_stuffing(unsigned char* string, int *length){
    unsigned char* new_string = (unsigned char*)malloc(*length);
    int new_length = *length;
    new_string[0] = string[0];

    for(int i = 1, j=1; i < *length-1; i++, j++){
        if(string[i] == FLAG){
            new_length++;
            new_string = realloc(new_string, new_length);
            new_string[j] = ESCAPE;
            j++;
            new_string[j] = 0x5E;
        }
        else if(string[i] == ESCAPE){
            new_length++;
            new_string = realloc(new_string, new_length);
            new_string[j] = ESCAPE;
            j++;
            new_string[j] = 0x5D;
         }
        else
        {
            new_string[j] = string[i];
        }
    }
    new_string[new_length-1] = string[*length-1];


    string = realloc(string, new_length);
    memcpy(string, new_string, new_length);
    *length = new_length;

    printf("\n\n");

    free(new_string);
}

/**
 * Byte destuffing
 */
void byte_destuffing(char* string, int *length){
    char* new_string = malloc(*length);
    int new_length = *length;

    for(int i = 0, j = 0; i < *length; i++, j++){
        if(string[i] == ESCAPE){
            if(string[i+1] == 0x5e) {
                new_string[j] = FLAG;
            }
            else {
                new_string[j] = ESCAPE;
            }

            new_length--;

            i++;
        }
        else
        {
            new_string[j] = string[i];
        }
    }
    //new_string = realloc(new_string, new_length);
    //string = realloc(string, new_length);
    string = memcpy(string, new_string, new_length);

    *length = new_length;

    free(new_string);
}

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
{
    // REMOVER depois porque a app_player nunca permitirá isto
    if (length <= 0)
    {
        write(stderr, "Length must be positive!\n", 26);
        return -1;
    }
    printf("Sender: %x; receiver: %x\n", sender_field, receiver_ready_field);

    void *sigalrm_handler = signal(SIGALRM, atende);
    
    int plot_length = length + 6; 
    unsigned char *plot = malloc(plot_length * sizeof(char));

    create_information_plot(sender_field, buffer, length, plot);
       
    byte_stuffing(plot, &plot_length);

    int size_written;
    unsigned char received_char;
    int state = 0;
    unsigned reject_counter = 0;

    do
    {
        alarm_flag = 1;
        counter = 0;
        state = 0;
        alarm(0);
        received_char = 0xFF;
        while (counter < 4 && state < 5)
        {
            printf("Transmission number %d\n", counter);
            if (alarm_flag)
            {
                alarm(timeout);
                printf("Set alarm!\n\n");
                alarm_flag = 0;
            }

            if ((size_written = write(fd, plot,  plot_length)) < 0)
            {
                perror("write");
                return -1;
            }

            printf("Plot written: ");
            for(int i = 0; i < plot_length; i++){
                printf("%x ", plot[i]);
            }
            printf("\n\n");

            printf("%d bytes written.\n", size_written);

            unsigned char RR[5];

            state = 0;

            while (state != 5) /* loop for input */
            {
                if (alarm_flag == 1)
                {
                    printf("Receiving error at retransmission number %d\n", counter - 1);
                    break;
                }

                if(read(fd, &RR[state], 1)<0){ /* returns after 5 chars have been input */
                    perror("read");
                }

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
                    if (RR[state] == receiver_ready_field)
                    {
                        received_char = receiver_ready_field;
                        printf("Received Receiver Ready\n\n");
                        state = 3;
                    }
                    else if (RR[state] == FLAG)
                        state = 1;
                    else //Error in transmission: receiver asked for the same plot
                    {
                        received_char = receiver_reject_field;
                        printf("Received Receiver Reject\n\n");
                        reject_counter++;
                        if(reject_counter >= MAX_REJECTS){
                            free(plot);

                            alarm(0);
                            (void)signal(SIGALRM, sigalrm_handler);
                            return -1;
                        }
                        state = 3;
                    }
                    break;
                case 3:
                    if (RR[state] == received_char ^ A)
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
        }
        if(counter >= 4){
          printf("Can't transmit plot.\n\n");
          return -1;
        }
    } while (received_char != receiver_ready_field);

    free(plot);

    alarm(0);
    (void)signal(SIGALRM, sigalrm_handler);

    update_transm_nums();

    return size_written;
}

int llread(int fd, char *buffer)
{
    printf("\nStarting llread\n");
    printf("Information plot %d\n\n", information_plot_counter);
    information_plot_counter++;

    unsigned char received_plot[256];
    int received_plot_length = 0;

    int reject_counter = 0;

    while(reject_counter < MAX_REJECTS)
    {
        // Receives the information plot
        if(receive_information_plot(fd, received_plot, &received_plot_length)) {
            printf("\nRejected plot, BCC1 is wrong\n\n");

            send_supervision_plot(fd, receiver_reject_field);

            reject_counter++;
            break;
        }

        // Destuffs the plot
        byte_destuffing(received_plot, &received_plot_length);        

        int data_length = 0;
        unsigned char* data = retrieve_data(received_plot, received_plot_length, &data_length);

        // Checks if the BCC2 is correct
        unsigned char received_bcc2 = retrieve_bcc2(received_plot, received_plot_length);
        unsigned char calculated_bcc2 = calculate_bcc2(data, data_length);
        if(received_bcc2 != calculated_bcc2) {
            printf("\nRejected plot, BCC2 is wrong\n\n");
            
            send_supervision_plot(fd, receiver_reject_field);

            reject_counter++;
            break;
        }
        
        send_supervision_plot(fd, receiver_ready_field);

        update_transm_nums();

        return data_length;
    }

    return -1;
}

int llclose(int fd, struct termios *oldtio)
{
    if (tcsetattr(fd, TCSANOW, oldtio) == -1)
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

void update_transm_nums() {
    if(sender_field == C_SEND_0) {
        sender_field = C_SEND_1;
        receiver_ready_field = C_RR_0;
        receiver_reject_field = C_REJ_0;
    }
    else
    {
        sender_field = C_SEND_0;
        receiver_ready_field = C_RR_1;
        receiver_reject_field = C_REJ_1;
    }
}

/* Plot-sending auxiliary functions */

unsigned char* create_supervision_plot(char control_field) {
    unsigned char *plot = malloc(SUPERVISION_PLOT_SIZE);  

    plot[0] = FLAG;
    plot[1] = A;
    plot[2] = control_field;
    plot[3] = plot[1] ^ plot[2];
    plot[4] = FLAG;

    return plot;
}

void create_information_plot(char control_field, char *data, int length, unsigned char* plot) {
    unsigned char BCC2 = 0;
    for(int i = 0; i < length; i++) {
        BCC2 ^= data[i];
    }

    plot[0] = FLAG;
    plot[1] = A;
    plot[2] = control_field;
    plot[3] = plot[1] ^ plot[2];
    memcpy(&plot[4], data, length);
	plot[4 + length] = BCC2;
	plot[5 + length] = FLAG;    
}

int receive_information_plot(int fd, unsigned char *received_plot, int *received_plot_length) {
    int state = 0;
    int last_state = 0;

    printf("\n");
    while (state != -1)
    {
        int num = read(fd, &received_plot[state], 1);

        switch (state)
        {
        case 0:
            printf("State 0: %x\n",received_plot[state]);
            if (received_plot[state] == FLAG) {
                state = 1;
            }
            break;
        case 1:
            printf("State 1: %x\n",received_plot[state]);
            if (received_plot[state] == A) {
                state = 2;
            }
            else if (received_plot[state] == FLAG)
                state = 1;
            else
                state = 0;
            break;
        case 2:
            printf("State 2: %x\n",received_plot[state]);
            if (received_plot[state] == sender_field) {
                state = 3;
            }
            else if (received_plot[state] == FLAG)
            {
                state = 1;
            }
            else
            {
                state = 0;
            }
            break;
        case 3:
            printf("State 3: %x\n",received_plot[state]);
            if (received_plot[state] == A ^ sender_field)  {
                state = 4;
            }
            else
            {   
                // BCC1 is wrong
                return -1;
            }
            break;

        default:
            printf("Default: %x\n",received_plot[state]);
            if(received_plot[state] == FLAG){
                state = -1;
                break;
            }
            
            state++;
            last_state = state;

            break;
        }
    }    

    *received_plot_length = last_state + 1;
    return 0;
}

int send_supervision_plot(int fd, char control_field) {
    unsigned char* receiver_response = create_supervision_plot(control_field);
    write(fd, receiver_response, 5);
    free(receiver_response);  

    return 0;  
}

unsigned char* retrieve_data(unsigned char *information_plot, int plot_length, int *data_length) {
    printf("AHAHAHAHA %d", *data_length);
    *data_length = plot_length - 6;
    
    unsigned char *data = (unsigned char*)malloc(*data_length);
    
    data = memcpy(data, &information_plot[4], *data_length);

    return data;
}

unsigned char retrieve_bcc2(unsigned char *information_plot, int plot_length) {
    return information_plot[plot_length - 2];
}

unsigned char calculate_bcc2(unsigned char *data, int data_length) {
    unsigned char bcc2 = 0;

    for(int i = 0; i < data_length; i++) {
        bcc2 ^= data[i];
    }

    return bcc2;
}


int send_plot(int fd, unsigned char* plot) {
    return 0;
}

void print_string(unsigned char *string, int length) {
    for(int i = 0; i < length; i++) {
        printf("%x ", string[i]);
    }
    printf("\n");
}
