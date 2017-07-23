#ifndef CAPTURE_H
#define CAPTURE_H

#include <pthread.h>

#include <exception>
#include <string>
using namespace std;

class CreateThreadException: public exception {
		const char* what();
};


class Capture {
	public:
		static void start() throw(CreateThreadException);
		static void stop();
	protected:
		Capture();
		void * run(void *params);
		bool running;
		static Capture * me;
		pthread_t thread;


};
#endif