#include "daemon.h"
#include "log.h"
#include "blueping.h"
#include "ping.h"
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <wiringPi.h>
#include <time.h>
#include "notify.h"
#include "camera.h"
#include "imagesbank.h"
#include "capture.h"

#define LED_PIN 1
#define PIR_PIN 0

class AlarmDaemon:public Daemon {
	public:
		static void Initialize(string program, string version, string description) {
			if (Daemon::me==NULL) {
				Daemon::me=new AlarmDaemon(program, version, description);
			}
		}
		~AlarmDaemon() {
		}
	protected :
		int lastevent;
		bool monitor;
		bool stateon;
		AlarmDaemon(string program, string version, string description):Daemon(program, version, description) {
			try {
				this->parameters->add("alertkey", "The ApiKey to send sms alert", true, "xx:xx:xx:xx:xx");
				this->parameters->add("alerturl", "The Url to send sms alert", true, "http://");
				this->parameters->add("bankkey", "The ApiKey for ImagesBank service", true, "xx:xx:xx:xx:xx");
				this->parameters->add("bankurl", "The Url of ImagesBank service", true, "http://");
				Log::logger->log("MAIN",NOTICE) << "Adding program parameters descriptions" << endl;
			} catch(ExistingParameterNameException &e ) {
				Log::logger->log("MAIN", EMERGENCY) << "Can't create one of the file parameters"<< endl;
			}
		}

		void daemon(){
			Log::logger->log("MAIN",NOTICE) << "Child daemon started" << endl;
			Notify::ApiKey=this->parameters->get("alertkey")->asString();
			Notify::ApiUrl=this->parameters->get("alerturl")->asString();
			ImagesBank::ApiKey=this->parameters->get("bankkey")->asString();
			ImagesBank::ApiUrl=this->parameters->get("bankurl")->asString();
			this->monitor=true;
			this->stateon=false;
			if (wiringPiSetup () < 0) {
				Log::logger->log("MAIN",ERROR) << "Failed to initialize wiringPi librairies: " << strerror (errno) << endl;
				this->monitor=false;
			} else {
				this->lastevent=(int) time(NULL);
				pinMode (PIR_PIN, INPUT) ; 
				pinMode (LED_PIN, OUTPUT);
				digitalWrite(LED_PIN, 0);
				if ( wiringPiISR (PIR_PIN, INT_EDGE_RISING, &AlarmDaemon::Intrusion) < 0 ) {
					Log::logger->log("MAIN",ERROR) << "Unable to setup ISR: " << strerror (errno) << endl;
					this->monitor=false;
				}
			}
			while (this->monitor) {
				int now=(int) time(NULL);
				if (lastevent+10<now) {
					if (this->stateon) {
						digitalWrite(LED_PIN, 0);
						Capture::stop();
						this->stateon=false;
					}
				}	
				delay( 1000 ); 
			}		
		}
		void terminate(){
			Log::logger->log("MAIN",NOTICE) << "Child daemon terminate" << endl;
			this->monitor=false;
			::system(this->parameters->get("stop")->asChars());
		}
		static void Intrusion(void) {
			AlarmDaemon * me=(AlarmDaemon *) Daemon::me;
			me->lastevent=(int) time(NULL);
			if (!me->stateon) {
				digitalWrite(LED_PIN, 1);
				Notify::notify(START_INTRUSION);
				Capture::start();
				me->stateon=true;
			}
		}	
};




int main(int argc, char **argv) {
	Log::logger->setLevel(DEBUG);
	AlarmDaemon::Initialize(argv[0], "1.0.0", "Alarm monitor with PIR mouvement detector");
		//exit(0);

	try {
		AlarmDaemon::Start(argc, argv);
	} catch(ForkException &e) {
		Log::logger->log("MAIN",NOTICE) << "ForkException occurs" << endl;
	} catch(OptionsStopException &e) {
		Log::logger->log("MAIN",NOTICE) << "OptionsStopException occurs" << endl;
	} catch(UnknownOptionException &e) {
		Log::logger->log("MAIN",NOTICE) << "UnknownException occurs" << endl;
	} catch(CantCreateFileException &e) {
		Log::logger->log("MAIN",NOTICE) << "CantCreateFileException occurs" << endl;
	}
}
