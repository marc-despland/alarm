#include "capture.h"
#include "log.h"
#include "camera.h"
#include "imagesbank.h"
#include <sstream>
#include <string.h>
#include <unistd.h>

#define JPEG_QUALITY 80
#define CAPTURE_INTERVAL 1

Capture * Capture::me=NULL;

Capture::Capture() {
	this->running=false;
}

void *Capture::run(void *params) {
	Log::logger->log("CAPTURE",DEBUG) << "Starting capture" << endl;
	Camera * camera=new Camera();
	try {
		camera->init();
	}catch(CameraOpenException &e) {
		Capture::me->running=false;
	}
	string collection=ImagesBank::createCollection();
	Log::logger->log("CAPTURE",DEBUG) << "Collection created : " <<collection<< endl;
	unsigned int index=0;
	while (Capture::me->running) {
		try {
			camera->capture();
		} catch(CameraOpenException &e) {}
		unsigned char * buffer;
		unsigned long size;
		std::stringstream image_name;
		image_name << "image" << index <<".jpg";
		buffer=camera->toJpeg(&size,JPEG_QUALITY);
		try {
			ImagesBank::upload(collection, image_name.str(), buffer, size);
		} catch(CurlInitException &e) {

		} catch(UploadException &e) {
			
		}
		index++;
		sleep(CAPTURE_INTERVAL);
	}
	delete camera;
	Log::logger->log("CAPTURE",DEBUG) << "Capture finished" << endl;
}

void Capture::start() throw(CreateThreadException) {
	if (Capture::me==NULL) Capture::me=new Capture();
	Capture::me->running=true;
	if(pthread_create(& Capture::me->thread, NULL, Capture::run, NULL)) {
		Log::logger->log("CAPTURE",ERROR) << "Can't create notifier thread " << strerror (errno) << endl;
		throw CreateThreadException();
	}
}

void Capture::stop() {
	if (Capture::me!=NULL) {
		Capture::me->running=false;
		pthread_join(Capture::me->thread, NULL);
	}
}
