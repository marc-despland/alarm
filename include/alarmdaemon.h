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
#include "istatus.h"

class AlarmDaemon: public Daemon, ILightController, IStatus {
	public:
		static void Initialize(string program, string version, string description);
		~AlarmDaemon();
		void lightOn();
		void lightOff();
		bool togglePause();
		string jsonStatus();
	protected:
		int lastevent;
		bool monitor;
		bool stateon;
		AlarmDaemon(string program, string version, string description);
		bool pause;
		void daemon();
		void terminate();
		static void Intrusion(void);
		string program;
		string version;
		string description;
};

#endif
