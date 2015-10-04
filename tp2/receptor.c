/*
* Non-Canonical Read
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
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x7E
#define ADDRESSE 0x03
#define ADDRESSR 0x01
#define CONTROLE 0x03
#define CONTROLR 0x07
#define BBCE ADDRESSE^CONTROLE
#define BBCR ADDRESSR^CONTROLR
volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res, counter;
    counter=0;
    struct termios oldtio,newtio;
    char buf[255],reply[255];
    
    // (1) Print Usage
    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }
    // --------------------------------------------------------------

    // (2) Serial Port configuration
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
    newtio.c_lflag = 0; /* set input mode (non-canonical, no echo,...) */
    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 chars received */
    tcflush(fd, TCIOFLUSH);
    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
    printf("New termios structure set\n");
    // ----------------------------------------------------------------->
	int estado=0;
	char readC;
	char setFrame[5];
	while(estado<5){
		read(fd,&readC,1);
		switch(estado){
			case 0:{ 
				if(readC==FLAG) estado++;
				break;
			}
			case 1:{
				if(readC==ADDRESSE) estado++;
				break;
			}
			case 2:{
				if(readC==CONTROLE) estado++;
				else if (readC==FLAG) estado=1;
				break;
			}
			case 3:{
				if(readC==BBCE) estado++;
				else if (readC==FLAG) estado=1;
				break;
			}
			case 4:{
				if(readC==FLAG) estado++;
				break;
			}
		}
	
	}
	printf("Receptor recebeu\n");
	setFrame[0]=FLAG;
	setFrame[1]=ADDRESSR;
	setFrame[2]=CONTROLR;
	setFrame[3]=BBCR;
	setFrame[4]=FLAG;
	write(fd,setFrame,5);
	printf("Receptor respondeu\n");
	sleep(3);
	
    // (4) Restore previous serial port configuratio
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    // ----------------------------------------------------------------->
    return 0;
}