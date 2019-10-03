/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x7e
#define A 0x03
#define C_SET 0x03

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
  int fd, res;
  struct termios oldtio,newtio;
  char buf[5];

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

  memset(&newtio, 0, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
  newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */



  /*
     VTIME e VMIN devem ser alterados de forma a proteger com um temporizador
     leitura do(s) pr�ximo(s) caracter(es)
     */



  tcflush(fd, TCIOFLUSH);

  if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");

  int state = 0;
  while (state != 5) {       /* loop for input */
    read(fd, &buf[state], 1);   /* returns after 5 chars have been input */
    switch (state) {
      case 0:
        if (buf[state]==FLAG) state = 1;
        break;
      case 1:
        if (buf[state]==A) state = 2;
        else if (buf[state]==FLAG) state = 1;
        else state = 0;
        break;
      case 2:
        if (buf[state]==C_SET) state = 3;
        else if (buf[state]==FLAG) state = 1;
        else state = 0;
        break;
      case 3:
        if (buf[state]==C_SET^A) state = 4;
        else state = 0;
        break;
      case 4:
        if (buf[state]==FLAG) state = 5;
        else state = 0;
        break;
      default:
        break;
    }
  }

  printf("%x %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3], buf[4]);

  /*
     O ciclo WHILE deve ser alterado de modo a respeitar o indicado no gui�o
     */


  res = write(fd, buf, 5);
  printf("%d bytes written\n", res);

  sleep(1);

  tcsetattr(fd,TCSANOW,&oldtio);
  close(fd);

  return 0;
}
