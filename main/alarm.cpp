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
#include "httpserver.h"

#define LED_PIN 1
#define PIR_PIN 0

#define JPEG_QUALITY 80

class RequestContext {
	public:
		RequestContext(HttpRequest * request, HttpResponse * response) {
			this->request=request;
			this->response=response;
		};
		~RequestContext() {
			delete this->request;
			delete this->response;
		}
		HttpRequest * request;
		HttpResponse * response;
};

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
				this->parameters->add("httpport", "The HTTP for the REST API", true, "8080");
				this->parameters->add("apikey", "The ApiKey use to request the REST API", true, "xxxxxxxxxxxxxxxxxxxxx");
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
		void start_httpd() {
			pthread_t httpd_thread;
			if(pthread_create(&httpd_thread, NULL, AlarmDaemon::httpd, this)) {
				Log::logger->log("MAIN",ERROR) << "Can't create httpd thread " << strerror (errno) << endl;
			} else {
				if(pthread_detach(httpd_thread)) {
					Log::logger->log("MAIN",ERROR) << "Can't detach httpd thread " << strerror (errno) << endl;
				}
			}
		}

		static void * httpd(void *daemon) {
			AlarmDaemon * me=(AlarmDaemon *) daemon;
			Log::logger->log("MAIN",DEBUG) << "Sarting HTTPD server" <<endl;
			HttpServer * server=new HttpServer(me->parameters->get("httpport")->asInt(),20, true);
			server->add(HTTP_GET, "/api/image", captureLiveImage);
			server->run();
			return NULL;
		}

		static int captureLiveImage(HttpRequest * request, HttpResponse * response) {
			string apikey="";
			try {
				apikey=request->getHeader("apikey");
			} catch(std::out_of_range &e) {}
			AlarmDaemon * me=(AlarmDaemon *) Daemon::me;
			if (apikey!=me->parameters->get("apikey")->asString()) {
				Log::logger->log("MAIN", DEBUG) << "Forbbiden request" <<endl;
				response->setStatusCode(403);
				response->setStatusMessage("FORBIDDEN");
				response->send();
				delete response;
				delete request;
			} else {
				pthread_t httpd_thread;
				RequestContext * context=new RequestContext(request, response);
				if(pthread_create(&httpd_thread, NULL, AlarmDaemon::sendCapturedImage, context)) {
					Log::logger->log("MAIN",ERROR) << "Can't create send image thread " << strerror (errno) << endl;
				} else {
					if(pthread_detach(httpd_thread)) {
						Log::logger->log("MAIN",ERROR) << "Can't detach send image thread " << strerror (errno) << endl;
					}
				}
			}
			return 0;
		}

		static void * sendCapturedImage(void * params) {
			RequestContext * context=(RequestContext *) params;
			char * buffer=NULL;
			unsigned long size=0;
			Camera * camera=new Camera();
			try {
				camera->init();
				try {
					camera->capture();
					buffer=(char *) camera->toJpeg(&size,JPEG_QUALITY);
					context->response->setStatusCode(200);
					context->response->setStatusMessage("OK");
					context->response->setContentType("image/jpeg");
					context->response->setBody(buffer, size);
				} catch(CameraOpenException &e) {
					context->response->setStatusCode(500);
					context->response->setStatusMessage("Internal Server Error");
					context->response->setContentType("application/json");
					context->response->setBody("{\"message\": \"Can't capture the image\"}", 38);
				}
			}catch(CameraOpenException &e) {
				context->response->setStatusCode(500);
				context->response->setStatusMessage("Internal Server Error");
				context->response->setContentType("application/json");
				context->response->setBody("{\"message\": \"Can't initialize the Camera\"}", 42);
			}	
			context->response->send();
			delete camera;
			delete context;
			if (size>0) {
				free(buffer);
			}
			return NULL;
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
