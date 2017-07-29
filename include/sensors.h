#ifndef SENSORS_H
#define SENSORS_H

#include <pthread.h>
#include <map>

#include "sensorsbank.h"
#include "pthreadexception.h"
using namespace std;


class Sensors {
	public:
		static void start(unsigned int readinterval, unsigned int postinterval) throw (ThreadCreateException);
		static void stop();
	protected:
		static Sensors * me;
		Sensors(unsigned int readinterval, unsigned int postinterval);
		~Sensors();
		static void * run(void * sensors);
		bool readSensors();
		bool running;
		unsigned int readinterval;
		unsigned int postinterval;
		pthread_t * thread;
		std::map<string, double> value;
		std::map<string, double> lastavg;
};

#endif