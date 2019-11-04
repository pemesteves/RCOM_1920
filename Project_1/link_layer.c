#include "link_layer.h"
#include "macros.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>

int alarm_flag = 1, counter = 1;
unsigned int timeout = 3;

unsigned char sender_field = C_SEND_0;
unsigned char receiver_ready_field = C_RR_1;
unsigned char receiver_reject_field = C_REJ_1;

int information_plot_counter = 1;



/****************************/
/**                        **/
/**     Main Functions     **/
/**                        **/
/****************************/

int llopen(char *gate, int flag, struct termios *oldtio)
{
    if(check_role(flag)) {
        printf("ERROR: Role must be TRANSMITTER or RECEIVER\n");
        return -1;
    }

    if(check_serial_port(gate)) {
        printf("ERROR: Wrong serial port\n");
        return -1;
    }

    int fd;
    if((fd = open_serial_port(gate)) < 0) {
        printf("ERROR: Unable to open serial port\n");
        return -1;
    }
    
    if(save_current_termios(fd, oldtio)){
        printf("ERROR: Unable to save current termios settings\n");
        return -1;
    }
    
    struct termios newtio;
    if(set_new_termios(fd, &newtio, flag)) {
        printf("ERROR: Unable to create new termios and set it\n");
        return -1;
    }
    
    void* sigalrm_handler;
    if(flag == TRANSMITTER){
        sigalrm_handler = signal(SIGALRM, atende);  // instala  rotina que atende interrupcao
    }

    bool connected = false;
    unsigned char *received_plot = (unsigned char *)malloc(255 * sizeof(unsigned char));
    
    switch(flag) {
    case TRANSMITTER:
        while(!connected) {

            if(counter > 4) {
                write(stderr, "Can't open protocol\n", 21);
                return -1;
            }
            
            printf("Transmission number %d\n", counter);
            if(alarm_flag) {
                alarm(newtio.c_cc[VTIME]);
                alarm_flag = 0;
            }

            send_supervision_plot(fd, C_SET);

            if(receive_supervision_plot(fd, received_plot, flag))
                continue;

            if(check_control_field(received_plot, C_UA))
                continue;

            connected = true;
        }
        break;

    case RECEIVER:

        while(!connected) {
            receive_supervision_plot(fd, received_plot, flag);

            if(check_control_field(received_plot, C_SET))
                continue;

            send_supervision_plot(fd, C_UA);

            connected = true;
        }

        break;
    }

    free(received_plot);

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

    // Install the alarm handler
    void *sigalrm_handler = signal(SIGALRM, atende);

    // Creates the information plot
    int plot_length = length + 6;
    unsigned char *plot = malloc(plot_length * sizeof(char));
    create_information_plot(sender_field, buffer, length, plot);

    // Stuffs it
    byte_stuffing(&plot, &plot_length);

    int size_written;
    unsigned char *received_plot = malloc(5 * sizeof(char));
    bool sending = true;

    while(sending) {
        reset_alarm();

        while(counter > 4) {
            printf("Transmission number %d\n", counter);
            if (alarm_flag) {
                alarm(timeout);
                printf("Set alarm!\n\n");
                alarm_flag = 0;
            }

            if ((size_written = write(fd, plot,  plot_length)) < 0) {
                perror("write");
                return -1;
            }
            
            if(receive_supervision_plot(fd, received_plot, TRANSMITTER))
                continue;

            if(check_control_field(received_plot, receiver_ready_field))
                continue;

            sending = true;
            break;
        }
        if(counter >= 4){
          printf("Can't transmit plot.\n\n");
          return -1;
        }

    }
    free(plot);

    alarm(0);
    (void)signal(SIGALRM, sigalrm_handler);

    update_trans_nums();

    return size_written;
}

int llread(int fd, char *buffer)
{
    printf("\nStarting llread\n");
    printf("Information plot %d\n\n", information_plot_counter);
    information_plot_counter++;

    unsigned char* received_plot = (unsigned char*)malloc(512*sizeof(unsigned char));
    int received_plot_length = 0;

    int reject_counter = 0;

    while(reject_counter < MAX_REJECTS)
    {
        // Receives the information plot
        if(receive_information_plot(fd, received_plot, &received_plot_length)) {
            printf("\nRejected plot, BCC1 is wrong\n\n");

            send_supervision_plot(fd, receiver_reject_field);

            reject_counter++;
            continue;
        }
        // Destuffs the plot
        byte_destuffing(&received_plot, &received_plot_length);

        int data_length = 0;
        unsigned char* data = retrieve_data(received_plot, received_plot_length, &data_length); // CHANGE! (buffer instead of data and passes to func params )

        // Checks if the BCC2 is correct
        unsigned char received_bcc2 = retrieve_bcc2(received_plot, received_plot_length);

        unsigned char calculated_bcc2 = calculate_bcc2(data, data_length);
        if(received_bcc2 != calculated_bcc2) {
            printf("\nRejected plot, BCC2 is wrong\n\n");

            send_supervision_plot(fd, receiver_reject_field);

            reject_counter++;
            continue;
        }

        send_supervision_plot(fd, receiver_ready_field);

        update_transm_nums();

        memcpy(buffer, data, data_length);
        free(data);
        free(received_plot);

        return data_length;
    }

    return -1;
}

int llclose(int fd, struct termios *oldtio, int flag)
{
    bool disconnected = false;
    unsigned char *received_plot = (unsigned char *)malloc(255 * sizeof(unsigned char));

    switch(flag) {
        case TRANSMITTER:
            while(!disconnected) {

                send_supervision_plot(fd, C_DISC);

                if(receive_supervision_plot(fd, received_plot, flag))
                    continue;
                if(check_control_field(received_plot, C_DISC))
                    continue;

                disconnected = true;
            }

            send_supervision_plot(fd, C_UA);

            break;

        case RECEIVER:
            while(!disconnected) {
                
                if(receive_supervision_plot(fd, received_plot, flag))
                    continue;
                if(check_control_field(received_plot, C_DISC))
                    continue;

                disconnected = true;
            }

            bool acknowledged = false;
            while(!acknowledged) {
                
                send_supervision_plot(fd, C_DISC);

                if(receive_supervision_plot(fd, received_plot, flag))
                    continue;
                if(check_control_field(received_plot, C_UA))
                    continue;

                acknowledged = true;
            }

            break;        
    }
    free(received_plot);
    
    if(flag == TRANSMITTER)
        sleep(2);

    if(set_termios(fd, oldtio)) {
        printf("ERROR: unable to set old termios\n");
        return -1;
    }

    printf("\nOld termios structure set at llclose\n");

    close(fd);
    return 0;   
}



/****************************/
/**                        **/
/**   Byte (De)Stuffing    **/
/**                        **/
/****************************/

void byte_stuffing(unsigned char** string, int *length){
    unsigned char* new_string = (unsigned char*)malloc(*length);
    int new_length = *length;
    new_string[0] = (*string)[0];

    for(int i = 1, j=1; i < *length-1; i++, j++){
        if((*string)[i] == FLAG){
            new_length++;
            new_string = realloc(new_string, new_length);
            new_string[j] = ESCAPE;
            j++;
            new_string[j] = 0x5E;
        }
        else if((*string)[i] == ESCAPE){
            new_length++;
            new_string = realloc(new_string, new_length);
            new_string[j] = ESCAPE;
            j++;
            new_string[j] = 0x5D;
         }
        else
        {
            new_string[j] = (*string)[i];
        }
    }
    new_string[new_length-1] = (*string)[*length-1];


    *string = realloc(*string, new_length);
    memcpy(*string, new_string, new_length);
    *length = new_length;
    
    free(new_string);
}

void byte_destuffing(unsigned char** string, int *length){
    unsigned char* new_string = (unsigned char*)malloc(*length);
    int new_length = *length;

    for(int i = 0, j = 0; i < *length; i++, j++){
        if((*string)[i] == ESCAPE){
            if((*string)[i+1] == 0x5e) {
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
            new_string[j] = (*string)[i];
        }
    }
    //new_string = realloc(new_string, new_length);
    //string = realloc(string, new_length);
    memcpy(*string, new_string, new_length);

    *length = new_length;

    free(new_string);
}



/****************************/
/**                        **/
/**   Supervision Plots    **/
/**                        **/
/****************************/

unsigned char* create_supervision_plot(char control_field) {
    unsigned char *plot = malloc(SUPERVISION_PLOT_SIZE);

    plot[0] = FLAG;
    plot[1] = A;
    plot[2] = control_field;
    plot[3] = plot[1] ^ plot[2];
    plot[4] = FLAG;

    return plot;
}

int send_supervision_plot(int fd, char control_field) {
    unsigned char* receiver_response = create_supervision_plot(control_field);
    write(fd, receiver_response, 5);
    free(receiver_response);

    return 0;
}

int receive_supervision_plot(int fd, unsigned char *received_plot, int flag) {
    //unsigned char *received_plot = (unsigned char*)malloc(MAX_SIZE * sizeof(unsigned char));

    int state = START_STATE;
    while (state != STOP_STATE)
    {
        if (flag == TRANSMITTER && alarm_flag == 1)
        {
            printf("Receiving error at retransmission number %d\n", counter - 1);
            return 1;
        }
        read(fd, &received_plot[state], 1); /* returns after 5 chars have been input */
        switch (state)
        {
        case START_STATE:
            if (received_plot[state] == FLAG)
                state = RECEIVED_FLAG;
            break;

        case RECEIVED_FLAG:
            if (received_plot[state] == A)
                state = RECEIVED_A;
            else if (received_plot[state] == FLAG)
                state = RECEIVED_FLAG;
            else
                state = START_STATE;
            break;

        case RECEIVED_A:
            if (valid_control_field(received_plot[state]))
                state = RECEIVED_CTRL;
            else if (received_plot[state] == FLAG)
                state = RECEIVED_FLAG;
            else
                state = START_STATE;
            break;

        case RECEIVED_CTRL:
            if (received_plot[state] == received_plot[1] ^ received_plot[2])
                state = CORRECT_BCC;
            else
                state = START_STATE;
            break;

        case CORRECT_BCC:
            if (received_plot[state] == FLAG)
                state = STOP_STATE;
            else
                state = START_STATE;
            break;

        default:
            break;
        }
    }

    return 0;
}



/****************************/
/**                        **/
/**   Information Plots    **/
/**                        **/
/****************************/

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

    bool bcc1_wrong = false;

    while (state != -1)
    {
        int num = read(fd, &received_plot[state], 1);

        switch (state)
        {
        case 0:
           // printf("State 0: %x\n",received_plot[state]);
            if (received_plot[state] == FLAG) {
                state = 1;
            }
            break;
        case 1:
          //  printf("State 1: %x\n",received_plot[state]);
            if (received_plot[state] == A) {
                state = 2;
            }
            else if (received_plot[state] == FLAG)
                state = 1;
            else
                state = 0;
            break;
        case 2:
           // printf("State 2: %x\n",received_plot[state]);
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
           // printf("State 3: %x\n",received_plot[state]);
            if (received_plot[state] == A ^ sender_field)  {
                state = 4;
            }
            else
            {
                // BCC1 is wrong
                bcc1_wrong = true;
                state = 4;
            }
            break;

        default:
           // printf("Default: %x\n",received_plot[state]);
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

    if(bcc1_wrong)
      return -1;

    return 0;
}



/****************************/
/**                        **/
/**     Control Field      **/
/**                        **/
/****************************/

bool check_control_field(unsigned char *plot, unsigned char control_field) {
    return plot[2] != control_field;
}

bool valid_control_field(unsigned char control_field) {
    if(control_field == C_SET    || control_field == C_REJ_0  || control_field == C_REJ_1 ||
       control_field == C_RR_0   || control_field == C_RR_1   || control_field == C_DISC  ||
       control_field == C_SEND_0 || control_field == C_SEND_1 || control_field == C_UA     )
       return 1;

    return 0;
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


/****************************/
/**                        **/
/**  Retrieval Functions   **/
/**                        **/
/****************************/

unsigned char* retrieve_data(unsigned char *information_plot, int plot_length, int *data_length) {
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



/****************************/
/**                        **/
/**   Serial Port Setup    **/
/**                        **/
/****************************/

int open_serial_port(char* port) {
	int fd;
    if((fd = open(port, O_RDWR | O_NOCTTY)) < 0){
        perror("open");
        return -1;
    }
    return fd;
}

int close_serial_port(int fd, struct termios *oldtio) {
	if (tcsetattr(fd, TCSANOW, oldtio) == -1) {
		perror("tcsetattr");
		return 1;
	}

	close(fd);
	return 0;
}

int set_termios(int fd, struct termios *newtio) {
    if (tcsetattr(fd, TCSANOW, newtio) != 0)
		return 1;

    return 0;
}

int set_new_termios(int fd, struct termios *newtio, int flag) {
	bzero(newtio, sizeof(*newtio));
	newtio->c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio->c_iflag = IGNPAR;
	newtio->c_oflag = 0;
	newtio->c_lflag = 0;

    switch(flag) {
        case TRANSMITTER:
            newtio->c_cc[VTIME] = 3; /* inter-character timer unused */
            newtio->c_cc[VMIN] = 0;  /* blocking read until 0 chars received */
            break;
        case RECEIVER:
            newtio->c_cc[VTIME] = 0; /* inter-character timer unused */
            newtio->c_cc[VMIN] = 1;  /* blocking read until 1 char received */
            break;
        default:
            printf("ERROR: flag doesn't exist!!!\n\n");
            return -1;
    }
    
	if (tcflush(fd, TCIOFLUSH) != 0)
		return 1;

	if (tcsetattr(fd, TCSANOW, newtio) != 0)
		return 1;

	printf("New termios structure set.\n");

	return 0;
}

int save_current_termios(int fd, struct termios *oldtio) {
	if (tcgetattr(fd, oldtio) != 0) {
		return 1;
	}

	return 0;
}

int check_role(int flag) {
    if (flag != TRANSMITTER && flag != RECEIVER)
        return -1;

    return 0;
}

int check_serial_port(char *port) {
    if ((strcmp("/dev/ttyS0", port) != 0) && (strcmp("/dev/ttyS1", port) != 0) && (strcmp("/dev/ttyS2", port) != 0) && (strcmp("/dev/ttyS4", port) != 0)) 
            return -1;

    return 0;
}



/****************************/
/**                        **/
/**    Alarm Functions     **/
/**                        **/
/****************************/

void atende(int signal)
{
    printf("alarme # %d\n", counter);
    alarm_flag = 1;
    counter++;
}

void reset_alarm() {
    alarm_flag = 1;
    counter = 0;
    alarm(0); 
}