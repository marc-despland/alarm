#ifndef ALARMDAEMON_H
#define ALARMDAEMON_H

#include "daemon.h"
#include "notify.h"
#include "camera.h"
#include "imagesbank.h"
#include "capture.h"
#include "httpd.h"
#include "sensors.h"
#include "ilightcontroller.h"

class AlarmDaemon: public Daemon, ILightController {
	public:
		static void Initialize(string program, string version, string description);
		~AlarmDaemon();
		void lightOn();
		void lightOff();
	protected:
		int lastevent;
		bool monitor;
		bool stateon;
		AlarmDaemon(string program, string version, string description);

		void daemon();
		void terminate();
		static void Intrusion(void);
};

#endif
