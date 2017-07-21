#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <wiringPi.h>
 
// Use GPIO Pin 17, which is Pin 0 for wiringPi library
 
#define BUTTON_PIN 0
 
// the event counter 
volatile int eventCounter = 0;
 
// -------------------------------------------------------------------------
 
void myInterrupt(void) {
	res=  digitalRead(0);
	if (res==0) {
		printf("no more movement detected .... back to sleep");
		eventCounter++
	} else {
		printf("/!\\ INTRUSION DETECTED !!!");
	}
}
 
// -------------------------------------------------------------------------
 
int main(void) {
  // sets up the wiringPi library
  if (wiringPiSetup () < 0) {
      fprintf (stderr, "Unable to setup wiringPi: %s\n", strerror (errno));
      return 1;
  }
  pinMode (BUTTON_PIN, INPUT) ; 
  // set Pin 17/0 generate an interrupt on high-to-low transitions
  // and attach myInterrupt() to the interrupt
  if ( wiringPiISR (BUTTON_PIN, INT_EDGE_BOTH, &myInterrupt) < 0 ) {
      fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
      return 1;
  }
 
  // display counter value every second.
  while ( 1 ) {
    printf( "%d\n", eventCounter );
    eventCounter = 0;
    delay( 1000 ); // wait 1 second
  }
 
  return 0;
}