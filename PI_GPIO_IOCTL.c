/*
 * User program that interacts with PI_GPIO device using IOCTL 
 * commands. 
 * 
 * */

#include "PI_GPIO.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>		
#include <sys/ioctl.h>  // ioctl
#include <unistd.h>	    // exit
#include <fcntl.h>		// open

#define CHOICE_BUF_MAX 10

/*
 * Functions for IOCL calls
 * 
 * */
 
 /*
  * @icotl_set_msg: IOCTl to send a message
  * 
  * */
ioctl_set_msg(int file_desc, char *message)
{

	int ret;
	ret = ioctl(file_desc, IOCTL_SET_MSG, message);
	if (ret < 0)
	{
		printf("ioctl set msg failed \n");
		exit(-1);
	}
	printf("PIN successfully written"); 
}

/*
 * @ioctl_get_msg: IOCTL get message
 * 
 * */
ioctl_get_msg(int file_desc){
	
	int ret; 
	char message[100];
	/*
	 * Kernel should be aware of buffer size, so it doesn't override
	 * it's buffer.
	 * 
	 * */
	 ret = ioctl(file_desc, IOCTL_GET_MSG, message);
	 if (ret < 0){
		printf("Ioctl get msg failed: %d \n",ret);
		exit(-1);
	 }
	 printf("Get message: %s ", message);  
	 printf("---------- Message done \n");

}

/*
 * @ioctl_get_nth_byte: two way, read and write ioctl function.
 * 
 * */
ioctl_get_nth_byte(int file_desc){
	
	int i;
	char c;
	printf("get nth byte");
	i=0;
	do {
		c = ioctl (file_desc, IOCTL_GET_NTH_BYTE, i++);
		if (c < 0){
			printf(" get nth byte failed \n");
		}
		putchar(c);
	}while ( c != 0);
	putchar('\n');
	
}

/*
 * @ioctl_get_nth_byte: two way, read and write ioctl function.
 * 
 * */
ioctl_read_nth_pin(int file_desc, char *gpio_pin){
	
	int ret;
	ret = ioctl (file_desc, IOCTL_READ_NTH_PIN, gpio_pin);
	if (ret < 0){
		printf(" get nth byte failed \n");
	}
	printf("%s \n",gpio_pin);
	
}

/*
 * @main
 * 
 * */
int main(){
	
	int file_desc, ret;
	char choice[CHOICE_BUF_MAX] ;
	char pin_number[CHOICE_BUF_MAX]; 
	file_desc = open(DEVICE_NAME,0);
	
	if (file_desc < 0 ) {
	
		printf("can't open device file \n");
		exit(-1);
	}
	
	printf("Device opened, Fd:%d \n", file_desc);
	while (1)
	{
		printf("\n Read or write to PINs, (r/w) \n");
		fflush(stdin);
		fgets(choice, CHOICE_BUF_MAX, stdin);		
		fflush(stdin);
		if (strncmp(choice,"r",1) == 0){
			printf("Enter Pin number \n");
			fflush(stdin);
			fgets(pin_number, CHOICE_BUF_MAX, stdin);		
			fflush(stdin);
			ioctl_read_nth_pin(file_desc, pin_number);
		}
		else {
			printf("Enter: {Pin,Value} \n");
			fflush(stdin);
			ret = (int)fgets(choice, CHOICE_BUF_MAX, stdin);
			ioctl_set_msg(file_desc, choice);	
		}
	}
	close(file_desc);
}
