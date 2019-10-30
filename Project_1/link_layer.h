#include <termios.h>

#define TRANSMITTER 0
#define RECEIVER 1

#define SUPERVISION_PLOT_SIZE 5

/* Main functions */
int llopen(char* porta, int flag, struct termios *oldtio);
int llwrite(int fd, char * buffer, int length);
int llread(int fd, char * buffer);
int llclose(int fd, struct termios *oldtio, int flag);

/* Serial port setup functions */
int open_serial_port(const char* port);
int close_serial_port(int fd, struct termios *oldtio);
int save_current_termios(int fd, struct termios *oldtio);
int set_new_termios(int fd, struct termios *newtio, int flag);
int check_role(int flag);
int check_serial_port(char *port);

/* Supervision plot functions */
unsigned char* create_supervision_plot(char control_field);
int receive_supervision_plot(int fd);
int send_supervision_plot(int fd, char control_field);

/* Information plot functions */
void create_information_plot(char control_field, char *data, int length, unsigned char* plot);
int receive_information_plot(int fd, unsigned char *received_plot, int *received_plot_length);

/* Calculation functions */
unsigned char* retrieve_data(unsigned char *information_plot, int plot_length, int *data_length);
unsigned char retrieve_bcc2(unsigned char *information_plot, int plot_length);
unsigned char calculate_bcc2(unsigned char *data, int data_length);
