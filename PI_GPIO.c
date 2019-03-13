/*
 * PI_GPIO.c: 
 *
*/

#include <linux/kernel.h>		// Needed for printk 
#include <linux/module.h> 		// All kernel modules need this
#include <linux/fs.h>			// linux file operation
#include <asm/uaccess.h>		// used for put user ??
#include <linux/uaccess.h>
#include <linux/gpio.h>			// GPIO kernel module		
#include <linux/moduleparam.h>

#include "PI_GPIO.h"


/*
 * Function prototypes 
 * 
 * */
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file*);
static int device_release(struct inode *, struct file*);
static ssize_t device_read (struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static long device_ioctl(struct file*,unsigned int, unsigned long);

#define SUCCESS 0
#define DEVICE_NAME_ "PI_GPIO"   // Dev name as it appears in /proc/devices
#define BUF_LEN 80			    // Max length of the message from the device
#define BASE_NUM 10
#define ASCII_OFFSET 48


MODULE_LICENSE("GPL";)

 
 // expose params
module_param(read_pin, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(read_pin, "An integer");
 
 
 // file operations 
 static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.unlocked_ioctl = device_ioctl,
	.open = device_open, 
	.release = device_release
}; 
 
 
 /*
  * @init_module, called when module is loaded. 
  * 
  * */
 
int init_module(void){
		int ret;
		ret = register_chrdev(Major, DEVICE_NAME_, &fops);
		
		if (ret < 0){
			printk(KERN_ALERT "DEV reg failed %d \n", Major);
			return Major;
		}
		
		printk(KERN_INFO "I was assigned major num %d , pin %d \n", Major,read_pin );
		printk(KERN_INFO "the driver create a dev file with \n");
		printk(KERN_INFO "mknod /dev/%s c %d 0 \n", DEVICE_NAME_, Major);
		printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
		printk(KERN_INFO "the device file.\n");
		printk(KERN_INFO "Remove the device file and module when done.\n");

		return SUCCESS;

}


/*
 * @cleanup: function called when module is unloaded
 * 
 * */
 void cleanup_module(void){
		/*
		 * Unregister device
		 * */      
		 unregister_chrdev(Major, DEVICE_NAME);		 	
}


/*
 * Methods
 * 
 * */

/*
 * @device_open: called when a proces tries to open the device file\
 * using cat /dev/mycharfile
 * 
 * */
static int device_open(struct inode *inode, struct file *file){
		
		int pin_value = gpio_get_value(read_pin);
		
		if (Device_Open) 
			return -EBUSY;
			
		Device_Open += 1;
		
		sprintf(msg, "New GPIO Reader opened, PIN_%d value is %d\n",gpio_pin,
			pin_value);
		msg_Ptr = msg; 
		try_module_get(THIS_MODULE);
		return SUCCESS; 

}


/*
 * @device_release called when a process closes the device file
 * 
 * */
static int device_release(struct inode *inode, struct file *file){
	Device_Open --;
	
	module_put(THIS_MODULE);
	
	return 0;
	 
}

/*
 * Called when a process attemps to read form the device
 *
 * */
static ssize_t device_read (struct file *filp, char *buffer, 
							size_t length, loff_t *offset){
	
	int bytes_read = 0;
	if (*msg_Ptr == 0 )
		return 0; 
	
	while (length && *msg_Ptr){
			/*
			 * The buffer is in the user data segment not in the kernel
			 * segment so * assignment won't work, we have to use 
			 * put user which copies data from the kernle data segment 
			 * to the user data segment
			 * 
			 * */
			 put_user (*(msg_Ptr++), buffer++);
			 length --;
			 bytes_read++;
	}
	
	printk(KERN_INFO  "New Device read called.\n",msg_Ptr );
	
	return bytes_read; 
	
}

/*
 * @power_of: A function that implements the power of operation
 * 
 * */
static int power_of(int base, int exponent){
	
	int i,ret_val = 1 ;
	
	for (i=0;i<exponent-1;i++)
		ret_val *= base;
	
	return ret_val; 
}


/*
 * @parse_line: Function to parse lines of characters to int
 * 
 * */
static int parse_line(char *line, int count){
	
	int i, ret_val =0, temp;
	
	for (i = 0; i < count; i++ ){
		if (*(line+i) == '\n') break;
		temp = ((int) *(line+i) ) - ASCII_OFFSET;
		if ( temp <= 0 || temp > 9) break;
		ret_val += temp * power_of(BASE_NUM, count-i) ;
	}
	return ret_val;
}


/*
 * @deivce_write: called when a process writes to dev file. Example when using 
 * echo "hello world" > /dev/hello
 * 
 * */
static ssize_t device_write (struct file *filp, const char *buff,
							size_t len, loff_t *off){
							
		
		int i,  num_digits = len; 
	
		copy_from_user (msg_Ptr, buff ,len);
		for (i =0; i < len; i++ ){
			num_digits = i;
			if ( *(msg_Ptr+i) == ',' ){
				break;
			}
		}
		// Get gpio pin number and output value
		gpio_pin = parse_line(msg_Ptr, num_digits);
		i    = parse_line(msg_Ptr+num_digits+1, len-1-num_digits-1);
		value = 0;
		if (i >0)
			value = 1;
		printk(KERN_INFO "New Setting pin %d value %d \n",gpio_pin, value);
		// set GPIO 
		gpio_direction_output(gpio_pin, value);
		//return -EINVAL;
		return len; 
}

/*
 * @device_ioctl: This function will be called when a process/program
 * does an ioctl on the device file. output of this function will be 
 * returned to the caller. 
 * 
 * Arguments: inode: structure to describe the file
 * 			  file*
 * 			  ioctl number and param
 * 
 * */
static long device_ioctl(struct file *file,
				 unsigned int ioctl_num, unsigned long ioctl_param){
	
	int i, pin, value; 
	char *temp;
	char ch;
	
	printk(KERN_INFO "Device IOCTL request received \n");
		
	if (ioctl_num == IOCTL_SET_MSG){

		// get the char pointer in user space and set that as
		// devices message. 
		temp = (char *) ioctl_param;
		get_user(ch, temp);
		for (i=0; ch && i < BUF_LEN; i++, temp++){
			get_user(ch, temp);
		}
		printk(KERN_INFO "Writing data : %s \n",ioctl_param);
		
		device_write(file, (char *)ioctl_param, i, 0);

	}
	else if ( ioctl_num == IOCTL_GET_MSG){

		// give the current message to the caller
		i = device_read(file, (char *)ioctl_param, 99, 0);
		// add end of seq symbol at the end
		put_user('\0', (char *)ioctl_param + i);

	}
	else if( ioctl_num ==  IOCTL_GET_NTH_BYTE){
		
		return msg[ioctl_param];

	}
	else if ( ioctl_num == IOCTL_READ_NTH_PIN){
		
		temp = (char *) ioctl_param;
		get_user(ch, temp);
		for (i=0; ch && i < BUF_LEN; i++, temp++){
			get_user(ch, temp);
		}
		pin = parse_line(ioctl_param, i-2);
		value = gpio_get_value(pin);
		sprintf(temp,"PIN_%d=>Value:%d\0", pin,value);
		copy_to_user(ioctl_param, temp, i+13 );
		
	}
	
	return SUCCESS;
	
}
