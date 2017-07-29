#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
 
// The PiWeather board i2c address
#define ADDRESS 0x04
 
// The I2C bus: This is for V2 pi's. For V1 Model B you need i2c-0
static const char *devName = "/dev/i2c-1";
 
int main(int argc, char** argv) {
 
  if (argc == 1) {
    printf("Supply one or more commands to send to the Arduino\n");
    exit(1);
  }
 
  printf("I2C: Connecting\n");
  int file;
 
  if ((file = open(devName, O_RDWR)) < 0) {
    fprintf(stderr, "I2C: Failed to access %d\n", devName);
    exit(1);
  }
 
  printf("I2C: acquiring buss to 0x%x\n", ADDRESS);
 
  if (ioctl(file, I2C_SLAVE, ADDRESS) < 0) {
    fprintf(stderr, "I2C: Failed to acquire bus access/talk to slave 0x%x\n", ADDRESS);
    exit(1);
  }
 
  int arg;
 
  for (arg = 1; arg < argc; arg++) {
    int val;
    unsigned char cmd[16];
 
    if (0 == sscanf(argv[arg], "%d", &val)) {
      fprintf(stderr, "Invalid parameter %d \"%s\"\n", arg, argv[arg]);
      exit(1);
    }
 
    printf("Sending %d\n", val);
 
    cmd[0] = val;
    if (write(file, cmd, 1) == 1) {
 
      // As we are not talking to direct hardware but a microcontroller we
      // need to wait a short while so that it can respond.
      //
      // 1ms seems to be enough but it depends on what workload it has
      usleep(10000);
 
      char buf[2];
      if (read(file, buf, 16) == 16) {
	float hum = ((float)(buf[0]+ (255*buf[1]))/10);
    	float temp = ((float)(buf[2]+ (255*buf[3]))/10);
        float mq2 = ((float)(buf[4]+ (255*buf[5]))/10);
        float mq4 = ((float)(buf[6]+ (255*buf[7]))/10); 
        float mq5 = ((float)(buf[8]+ (255*buf[9]))/10);
        float mq6 = ((float)(buf[10]+ (255*buf[11]))/10);
        float mq7 = ((float)(buf[12]+ (255*buf[13]))/10);
        float mq135 = ((float)(buf[14]+ (255*buf[15]))/10);
    	printf("Humidity 	%f\n", hum);
	printf("Temperature 	%f\n",temp);
	printf("MQ-2		%f\n",mq2);
	printf("MQ-4            %f\n",mq4);
	printf("MQ-5            %f\n",mq5);
	printf("MQ-6            %f\n",mq6);
	printf("MQ-7            %f\n",mq7);
	printf("MQ-135          %f\n",mq135);
      }
    }
 
    // Now wait else you could crash the arduino by sending requests too fast
    usleep(10000);
  }
 
  close(file);
  return (EXIT_SUCCESS);
}

