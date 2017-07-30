#include "alarmdaemon.h"
#include "log.h"

#include <wiringPi.h>
#include <time.h>


#define LED_PIN 1
#define PIR_PIN 0

void AlarmDaemon::Initialize(string program, string version, string description) {
	if (Daemon::me==NULL) {
		Daemon::me=new AlarmDaemon(program, version, description);
	}
}
AlarmDaemon::~AlarmDaemon() {
}
		
AlarmDaemon::AlarmDaemon(string program, string version, string description):Daemon(program, version, description) {
	try {
		this->parameters->add("httpport", "The HTTP for the REST API", true, "8080");
		this->parameters->add("apikey", "The ApiKey use to request the REST API", true, "xxxxxxxxxxxxxxxxxxxxx");
		this->parameters->add("alertkey", "The ApiKey to send sms alert", true, "xx:xx:xx:xx:xx");
		this->parameters->add("alerturl", "The Url to send sms alert", true, "http://");
		this->parameters->add("imageskey", "The ApiKey for ImagesBank service", true, "xx:xx:xx:xx:xx");
		this->parameters->add("imagesurl", "The Url of ImagesBank service", true, "http://");
		this->parameters->add("sensorskey", "The ApiKey for SensorsBank service", true, "xx:xx:xx:xx:xx");
		this->parameters->add("sensorsurl", "The Url of SensorsBank service", true, "http://");
		this->parameters->add("readinterval", "Delay in seconds between two sensor reading", false, "1");
		this->parameters->add("postinterval", "Delay in seconds between two post of sensors data", false, "900");

		Log::logger->log("ALARMDAEMON",NOTICE) << "Adding program parameters descriptions" << endl;
	} catch(ExistingParameterNameException &e ) {
		Log::logger->log("ALARMDAEMON", EMERGENCY) << "Can't create one of the file parameters"<< endl;
	}
	this->pause=false;
}

void AlarmDaemon::daemon(){
	Log::logger->log("ALARMDAEMON",NOTICE) << "Child daemon started" << endl;
	Httpd::ApiKey=this->parameters->get("apikey")->asString();
	Notify::ApiKey=this->parameters->get("alertkey")->asString();
	Notify::ApiUrl=this->parameters->get("alerturl")->asString();
	ImagesBank::ApiKey=this->parameters->get("imageskey")->asString();
	ImagesBank::ApiUrl=this->parameters->get("imagesurl")->asString();
	SensorsBank::ApiKey=this->parameters->get("sensorskey")->asString();
	SensorsBank::ApiUrl=this->parameters->get("sensorsurl")->asString();
	this->monitor=true;
	this->stateon=false;
	if (wiringPiSetup () < 0) {
		Log::logger->log("ALARMDAEMON",ERROR) << "Failed to initialize wiringPi librairies: " << strerror (errno) << endl;
		this->monitor=false;
	} else {
		this->lastevent=(int) time(NULL);
		pinMode (PIR_PIN, INPUT) ; 
		pinMode (LED_PIN, OUTPUT);
		digitalWrite(LED_PIN, 0);
		if ( wiringPiISR (PIR_PIN, INT_EDGE_RISING, &AlarmDaemon::Intrusion) < 0 ) {
			Log::logger->log("ALARMDAEMON",ERROR) << "Unable to setup ISR: " << strerror (errno) << endl;
			this->monitor=false;
		}
		Httpd::start(this->parameters->get("httpport")->asInt(), this);
		Sensors::start(this->parameters->get("readinterval")->asInt(),this->parameters->get("postinterval")->asInt());
	}
	this->pause=false;
	while (this->monitor) {
		int now=(int) time(NULL);
		if (lastevent+10<now) {
			if (this->stateon) {
				if (!this->pause) digitalWrite(LED_PIN, 0);
				Capture::stop();
				this->stateon=false;
			}
		}	
		delay( 1000 ); 
	}		
}
void AlarmDaemon::terminate(){
	Log::logger->log("ALARMDAEMON",NOTICE) << "Child daemon terminate" << endl;
	this->monitor=false;
	Httpd::stop();
	Sensors::stop();
}

void AlarmDaemon::Intrusion(void) {
	AlarmDaemon * me=(AlarmDaemon *) Daemon::me;
	me->lastevent=(int) time(NULL);
	if ((!me->stateon) && (!me->pause)) {
		digitalWrite(LED_PIN, 1);
		Notify::notify(START_INTRUSION);
		Capture::start();
		me->stateon=true;
	}
}


bool AlarmDaemon::togglePause() {
	AlarmDaemon * me=(AlarmDaemon *) Daemon::me;
	if (me->pause) {
		me->pause=false;
		if (!me->stateon) digitalWrite(LED_PIN, 0);
	} else {
		me->pause=true;
		digitalWrite(LED_PIN, 1);
	}
	return(me->pause);
}

void AlarmDaemon::lightOn() {
	if (!this->stateon) {
		digitalWrite(LED_PIN, 1);
		usleep(200000);
	}
}
void AlarmDaemon::lightOff() {
	if (!this->stateon) {
		digitalWrite(LED_PIN, 0);
	}

}