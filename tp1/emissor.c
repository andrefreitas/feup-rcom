/*
* Non-Canonical Writing
* Original code from RCOM
* Modified by André Freitas
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1



volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res,lenInput;
    struct termios oldtio,newtio;
    char buf[255],reply[255];
    int i, sum = 0, speed = 0, counter=0;
    
    // (1) Print Usage
    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }
    // ----------------------------------------------------------------->

    // (2) Serial Port configuration
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }
    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);}
    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0; 	/* set input mode (non-canonical, no echo,...) */
    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */
    tcflush(fd, TCIOFLUSH);
    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);}
    printf("New termios structure set\n");
    // ----------------------------------------------------------------->

    // (3) Send/Receive Data
    printf("Message: ");
    gets(buf);
    lenInput=strlen(buf);
    printf("You wrote: \"%s\" with %d chars\n",buf,lenInput);

    // (msg) ----> receiver
    write(fd,buf,lenInput+1);
    
    // (reply) ----> source
    while (STOP==FALSE) {       /* loop for input */
      res = read(fd,buf,1);   /* returns after 1 char have been input */
      if (buf[0]=='\0') STOP=TRUE;
      reply[counter]=buf[0]; 
      counter++; // WARNING: counter includes the \0
    }
    printf("Echo: \"%s\" with %d chars \n\n", reply,counter-1);
   
    // (4) Restore previous serial port configuration
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
    close(fd);
    // ----------------------------------------------------------------->

    return 0;
}
