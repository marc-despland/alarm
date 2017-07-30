#include "httpd.h"
#include "log.h"

#define JPEG_QUALITY 80

string Httpd::ApiKey="";

Httpd * Httpd::me=NULL;

void Httpd::start(int port, ILightController * light, IStatus * status) throw (ThreadCreateException){
	if (Httpd::me==NULL) {
		Httpd::me=new Httpd(port, light,status);
		Httpd::me->running=true;
		if(pthread_create(Httpd::me->thread, NULL, Httpd::run, Httpd::me)) {
			Log::logger->log("HTTPD",ERROR) << "Can't create HTTPD thread " << strerror (errno) << endl;
			throw ThreadCreateException();
		}
	}
}


void Httpd::stop() {
	Httpd::me->running=false;
	pthread_join(*(Httpd::me->thread), NULL);
	delete Httpd::me;
	Httpd::me=NULL;
}

void * Httpd::run(void * httpd) {
	Httpd * me=(Httpd *) httpd;
	Log::logger->log("HTTPD",DEBUG) << "Sarting HTTPD server" <<endl;
	me->server=new HttpServer(me->port,20, true);
	me->server->add(HTTP_GET, "/api/image", Httpd::captureLiveImage);
	me->server->add(HTTP_GET, "/api/pause", Httpd::togglePause);
	me->server->add(HTTP_GET, "/api/status", Httpd::getStatus);
	me->server->run();
	return NULL;
}

Httpd::Httpd(int port, ILightController * light, IStatus * status) {
	this->port=port;
	this->light=light;
	this->appstatus=status;
	this->thread= (pthread_t *) malloc(sizeof(pthread_t));
}


Httpd::~Httpd() {
	free(this->thread);
}


int Httpd::togglePause(HttpRequest * request, HttpResponse * response) {
	string apikey="";
	try {
		apikey=request->getHeader("apikey");
	} catch(std::out_of_range &e) {}
	Httpd * me=Httpd::me;
	if (apikey!=Httpd::ApiKey) {
		Log::logger->log("HTTPD", DEBUG) << "Forbbiden request" <<endl;
		response->setStatusCode(403);
		response->setStatusMessage("FORBIDDEN");
		response->send();
		delete response;
		delete request;
	} else {
		Httpd * me=Httpd::me;
		bool state=me->light->togglePause();
		response->setStatusCode(200);
		response->setStatusMessage("OK");
		response->setContentType("application/json");
		if (state) {
			response->setBody("{\"pause\": true}", 15);
		} else {
			response->setBody("{\"pause\": false}", 16);
		}
		response->send();
		delete response;
		delete request;
	}
	return 0;
}

int Httpd::getStatus(HttpRequest * request, HttpResponse * response) {
	string apikey="";
	try {
		apikey=request->getHeader("apikey");
	} catch(std::out_of_range &e) {}
	Httpd * me=Httpd::me;
	if (apikey!=Httpd::ApiKey) {
		Log::logger->log("HTTPD", DEBUG) << "Forbbiden request" <<endl;
		response->setStatusCode(403);
		response->setStatusMessage("FORBIDDEN");
		response->send();
		delete response;
		delete request;
	} else {
		Httpd * me=Httpd::me;
		string status=me->appstatus->jsonStatus();
		response->setStatusCode(200);
		response->setStatusMessage("OK");
		response->setContentType("application/json");
		response->setBody(status.c_str(), status.length());
		response->send();
		delete response;
		delete request;
	}
	return 0;
}


int Httpd::captureLiveImage(HttpRequest * request, HttpResponse * response) {
	string apikey="";
	try {
		apikey=request->getHeader("apikey");
	} catch(std::out_of_range &e) {}
	Httpd * me=Httpd::me;
	if (apikey!=Httpd::ApiKey) {
		Log::logger->log("HTTPD", DEBUG) << "Forbbiden request" <<endl;
		response->setStatusCode(403);
		response->setStatusMessage("FORBIDDEN");
		response->send();
		delete response;
		delete request;
	} else {
		pthread_t httpd_thread;
		HttpContext * context=new HttpContext(request, response);
		if(pthread_create(&httpd_thread, NULL, Httpd::sendCapturedImage, context)) {
			Log::logger->log("HTTPD",ERROR) << "Can't create send image thread " << strerror (errno) << endl;
		} else {
			if(pthread_detach(httpd_thread)) {
				Log::logger->log("HTTPD",ERROR) << "Can't detach send image thread " << strerror (errno) << endl;
			}
		}
	}
	return 0;
}

void * Httpd::sendCapturedImage(void * params) {
	HttpContext * context=(HttpContext *) params;
	char * buffer=NULL;
	unsigned long size=0;
	Httpd * me=Httpd::me;
	me->light->lightOn();
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
	me->light->lightOff();
	context->response->send();
	delete camera;
	delete context;
	if (size>0) {
		free(buffer);
	}
	return NULL;
}