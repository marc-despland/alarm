#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <time.h>

// Use GPIO Pin 17, which is Pin 0 for wiringPi library

#define PIR_PIN 0
#define LED_PIN 1

// the event counter 
volatile int eventCounter = 0;
int lastevent=0;
// -------------------------------------------------------------------------

void myInterrupt(void) {
	int res=  digitalRead(PIR_PIN);
	printf("Value : %d\n",res);
	if (res==0) {
		printf("no more movement detected .... back to sleep\n");
		eventCounter++;
	} else {
		lastevent=(int) time(NULL);
		digitalWrite(LED_PIN, 1);
		printf("/!\\ INTRUSION DETECTED !!!\n");
	}
}

// -------------------------------------------------------------------------

int main(void) {
// sets up the wiringPi library
	if (wiringPiSetup () < 0) {
		fprintf (stderr, "Unable to setup wiringPi: %s\n", strerror (errno));
		return 1;
	}
	lastevent=(int) time(NULL);
	pinMode (PIR_PIN, INPUT) ; 
	pinMode (LED_PIN, OUTPUT);
	digitalWrite(LED_PIN, 0);
// set Pin 17/0 generate an interrupt on high-to-low transitions
// and attach myInterrupt() to the interrupt
	if ( wiringPiISR (PIR_PIN, INT_EDGE_BOTH, &myInterrupt) < 0 ) {
		fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
		return 1;
	}

// display counter value every second.
	while ( 1 ) {
		int now=(int) time(NULL);
		if (lastevent+10<now) {
			digitalWrite(LED_PIN, 0);
		}	
		delay( 1000 ); // wait 1 second
	}

	return 0;
}