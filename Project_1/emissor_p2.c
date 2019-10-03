/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x7E
#define A 0x03
#define C_SET 0x03

volatile int STOP=FALSE;

int flag=1, counter=1;

/*
 * Signal Handler for SIGALRM
 */
void atende()                   // atende alarme
{
  printf("alarme # %d\n", counter);
  flag=1;
  counter++;
}

int main(int argc, char** argv)
{
  (void) signal(SIGALRM, atende);  // instala  rotina que atende interrupcao

  int fd,c, res;
  struct termios oldtio,newtio;
  char buf[255];
  int i, sum = 0, speed = 0;
  
  if ( (argc < 2) || 
       ((strcmp("/dev/ttyS0", argv[1])!=0) && 
        (strcmp("/dev/ttyS1", argv[1])!=0) )) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

  fd = open(argv[1], O_RDWR | O_NOCTTY );
  if (fd <0) {perror(argv[1]); exit(-1); }

  if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
    perror("tcgetattr");
    exit(-1);
  }

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME]    = 3;   /* inter-character timer unused */
  newtio.c_cc[VMIN]     = 0;   /* blocking read until 0 chars received */


  tcflush(fd, TCIOFLUSH);

  if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }
  printf("New termios structure set\n");

  unsigned char SET[5]; //[FLAG, A, C_SET, BCC, FLAG]

  int state = 0;  
  while(counter < 4 && state < 5){
    printf("Transmission number %d\n", counter);
    if(flag){
      alarm(newtio.c_cc[VTIME]);                 
      flag=0;
    }

    SET[0] = FLAG;
    SET[1] = A;
    SET[2] = C_SET;
    SET[3] = SET[1]^SET[2]; //BCC
    SET[4] = FLAG;
    
    if(write(fd, SET, 5) < 0){
      perror("write");
      exit(-1);
    }

    memset(SET, '\0', sizeof(SET));

    state = 0;
    while (state != 5) {       /* loop for input */
      if(flag == 1){
        printf("Receiving error at retransmission number %d\n", counter-1);
        break;  
      }
      read(fd, &SET[state], 1);   /* returns after 5 chars have been input */
      switch (state) {
        case 0:
          if (SET[state]==FLAG) state = 1;
          break;
        case 1:
          if (SET[state]==A) state = 2;
          else if (SET[state]==FLAG) state = 1;
          else state = 0;
          break;
        case 2:
          if (SET[state]==C_SET) state = 3;
          else if (SET[state]==FLAG) state = 1;
          else state = 0;
          break;
        case 3:
          if (SET[state]==C_SET^A) state = 4;
          else state = 0;
          break;
        case 4:
          if (SET[state]==FLAG) state = 5;
          else state = 0;
          break;
        default:
          break;
      }
    }
  }
  printf("%x %x %x %x %x\n", SET[0], SET[1], SET[2], SET[3], SET[4]);


  if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }
  printf("New termios structure set\n");
  
  close(fd);
  return 0;
}
