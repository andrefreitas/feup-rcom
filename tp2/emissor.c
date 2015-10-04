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
#include <signal.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
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
char setFrame[5];
int fd, count=0;
void atende(){
	count ++;
	if(count<=3){ // Nº de tentativas
		printf("Alarme%d\n",count);
		write(fd,setFrame,5);
		alarm(3);
	}
	else exit(0);
}

int main(int argc, char** argv)
{
    int c, res,lenInput;
    struct termios oldtio,newtio;
    char buf[255],reply[255];
    int i, sum = 0, speed = 0, counter=0;
	
	(void) signal(SIGALRM,atende);
    
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
    newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */
    tcflush(fd, TCIOFLUSH);
    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);}
    printf("New termios structure set\n");
    // ----------------------------------------------------------------->
	int estado=0;
	char readC;
	setFrame[0]=FLAG;
	setFrame[1]=ADDRESSE;
	setFrame[2]=CONTROLE;
	setFrame[3]=BBCE;
	setFrame[4]=FLAG;
	write(fd,setFrame,5);
	printf("Emissor escreveu\n");
	alarm(3);
	while(estado<5){
		read(fd,&readC,1);
		switch(estado){
			case 0:{ 
				if(readC==FLAG) estado++;
				break;
			}
			case 1:{
				if(readC==ADDRESSR) estado++;
				break;
			}
			case 2:{
				if(readC==CONTROLR) estado++;
				else if (readC==FLAG) estado=1;
				break;
			}
			case 3:{
				if(readC==BBCR) estado++;
				else if (readC==FLAG) estado=1;
				break;
			}
			case 4:{
				if(readC==FLAG) estado++;
				break;
			}
		}
	
	}
	alarm(0);
	printf("Emissor recebeu o UA\n");
  
    // (4) Restore previous serial port configuration
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
    close(fd);
    // ----------------------------------------------------------------->

    return 0;
}
