

#ifndef PI_GPIO_H
#define PI_GPIO_H

#include <linux/ioctl.h>


#define SUCCESS 0
#define  DEVICE_NAME "/dev/PI_GPIO"  // Dev name as it appears in /proc/devices
#define BUF_LEN 80			// Max length of the message from the device
#define BASE_NUM 10
#define ASCII_OFFSET 48

/*
 * Global variables are declared as static, so are global within the file
 * */
 
static int Device_Open = 0;    // Used to prevent multiple acces to driver
static int read_pin = 0;
static char msg[BUF_LEN];
static char *msg_Ptr; 
static int gpio_pin;
static int value; 
static int Major = 243;    // Major number assigned to our device driver
 
// Set the message of the driver
#define IOCTL_SET_MSG _IOR(Major, 0, char *)
        
//get the message of the device
#define IOCTL_GET_MSG _IOR(Major,1, char *)

// get nth byte. This works as both read and write
// or is used as both input and output
#define IOCTL_GET_NTH_BYTE _IOWR(Major, 2, int)

// get nth byte. This works as both read and write
// or is used as both input and output
#define IOCTL_READ_NTH_PIN _IOWR(Major, 3, char *)

#endif
