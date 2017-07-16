#include <stdio.h>
#include <wiringPi.h>
 
int main (void)
{
  int res;
  printf ("Raspberry Pi blink\n") ;
 
  if (wiringPiSetup () == -1)
    return 1 ;
 
  pinMode (0, INPUT) ;         // aka BCM_GPIO pin 17
  for (;;) { 
  	res=  digitalRead(0);
  	printf("Result : %d\n",res);
	delay(500);
  }
  return 0 ;
}
