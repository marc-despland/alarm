#include "daemon.h"
#include "log.h"
#include "blueping.h"
#include "ping.h"
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>


#include "alarmdaemon.h"






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
