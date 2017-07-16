#include <stdio.h>
#include <wiringPi.h>
 
int main (void)
{
  printf ("Raspberry Pi blink\n") ;
 
  if (wiringPiSetup () == -1)
    return 1 ;
 
  pinMode (0, OUTPUT) ;         // aka BCM_GPIO pin 17
 
    digitalWrite(0,0);
  return 0 ;
}
