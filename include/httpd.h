#ifndef HTTPD_H
#define HTTPD_H

#include <pthread.h>
#include "pthreadexception.h"
#include "httpcontext.h"
#include "httpserver.h"
#include "camera.h"
#include "ilightcontroller.h"

using namespace std;


class Httpd {
	public:
		static void start(int port, ILightController * light) throw (ThreadCreateException);
		static void stop();
		static string ApiKey;
	protected:
		static Httpd * me;
		Httpd(int port, ILightController * light);
		~Httpd();
		static void * run(void * httpd);
		static int togglePause(HttpRequest * request, HttpResponse * response);
		static int captureLiveImage(HttpRequest * request, HttpResponse * response);
		static void * sendCapturedImage(void * params);
		bool running;
		pthread_t * thread;
		int port;
		HttpServer * server;
		ILightController * light;
};

#endif