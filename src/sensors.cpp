#include "sensors.h"
#include "sensorsbank.h"

#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string>
#include <sstream>
#include <string.h>
#include <unistd.h>
#include "log.h"

// db.sensor_Temperature.aggregate([{ $match: {"month": 6}},{$group: { _id: "$hours", temperature: { $avg : "$value" }}}]);
Sensors * Sensors::me=NULL;
#define ADDRESS 0x04
 
// The I2C bus: This is for V2 and V3 pi's. For V1 Model B you need i2c-0
static const char *devName = "/dev/i2c-1";

std::map<string, double> Sensors::data() {
	return (Sensors::me->lastavg);
}

void Sensors::start(unsigned int readinterval, unsigned int postinterval) throw (ThreadCreateException){
	if (Sensors::me==NULL) {
		Sensors::me=new Sensors(readinterval,postinterval);
		Sensors::me->running=true;
		if(pthread_create(Sensors::me->thread, NULL, Sensors::run, Sensors::me)) {
			Log::logger->log("SENSORS",ERROR) << "Can't create sensors thread " << strerror (errno) << endl;
			throw ThreadCreateException();
		}
	}
}


void Sensors::stop() {
	Sensors::me->running=false;
	pthread_join(*(Sensors::me->thread), NULL);
	delete Sensors::me;
	Sensors::me=NULL;
}

void * Sensors::run(void * sensors) {
	Sensors * me=(Sensors *) sensors;
	unsigned int postloop=me->postinterval/me->readinterval;
	unsigned int loop=0;
	while (me->running) {
		sleep(me->readinterval);
		if (me->readSensors()) {
			loop++;
			if (loop>postloop) {
				for (std::map<string, double>::iterator it=me->value.begin();it!=me->value.end(); it++) {
					me->lastavg[it->first]=it->second/loop;
					me->value[it->first]=0;
				}
				try {
					SensorsBank::sendData(me->lastavg);
				} catch(CurlInitException &e) {
				} catch(PostException &e) {
				}	
				loop=0;
			}
		}
	}
	return NULL;
}

Sensors::Sensors(unsigned int readinterval, unsigned int postinterval) {
	this->readinterval=readinterval;
	this->postinterval=postinterval;
	this->thread= (pthread_t *) malloc(sizeof(pthread_t));
	this->value["Temperature"]=0;
	this->value["Humidity"]=0;
	this->value["MQ-2"]=0;
	this->value["MQ-4"]=0;
	this->value["MQ-5"]=0;
	this->value["MQ-6"]=0;
	this->value["MQ-7"]=0;
	this->value["MQ-135"]=0;
	this->lastavg["Temperature"]=0;
	this->lastavg["Humidity"]=0;
	this->lastavg["MQ-2"]=0;
	this->lastavg["MQ-4"]=0;
	this->lastavg["MQ-5"]=0;
	this->lastavg["MQ-6"]=0;
	this->lastavg["MQ-7"]=0;
	this->lastavg["MQ-135"]=0;

}


Sensors::~Sensors() {
	free(this->thread);
}


bool Sensors::readSensors() {
	int file;
	if ((file = open(devName, O_RDWR)) < 0) {
    	Log::logger->log("SENSORS",ERROR) << "Failed to access I2C device " << devName << endl;
    	return false;
	}
 
	Log::logger->log("SENSORS",DEBUG) << "I2C: acquiring buss to 0x" << ADDRESS << endl;
 	if (ioctl(file, I2C_SLAVE, ADDRESS) < 0) {
    	Log::logger->log("SENSORS",ERROR) << "I2C: Failed to acquire bus access/talk to slave 0x" << ADDRESS << endl;
    	close(file);
		return false;
	}
 	unsigned char cmd[1];
 	bool result=true;
    cmd[0] = 0;
    if (write(file, cmd, 1) == 1) {
		usleep(10000);
		char buf[17];
		if (read(file, buf, 17) == 17) {
			char crc=0;
			for (int i=0;i<16;i++) crc=crc ^ buf[i];
			if (crc==buf[16]) {
		        this->value["Humidity"]		+= ((double) (buf[0]+ (255*buf[1])))/10;
		    	this->value["Temperature"]	+= ((double) (buf[2]+ (255*buf[3])))/10;
		        this->value["MQ-2"] 		+= ((double) (buf[4]+ (255*buf[5])))/10;
		        this->value["MQ-4"]		 	+= ((double) (buf[6]+ (255*buf[7])))/10; 
		        this->value["MQ-5"]			+= ((double) (buf[8]+ (255*buf[9])))/10;
		        this->value["MQ-6"]			+= ((double) (buf[10]+ (255*buf[11])))/10;
		        this->value["MQ-7"]			+= ((double) (buf[12]+ (255*buf[13])))/10;
		        this->value["MQ-135"]		+= ((double) (buf[14] + (255*buf[15])))/10;

		    } else {
		    	Log::logger->log("SENSORS",DEBUG) << "Sensors Error Invalid CRC: " << (unsigned int) crc << " vs " << (unsigned int) buf[16] << endl;
		    	result=false;
		    }
		} else {
			Log::logger->log("SENSORS",DEBUG) << "Sensors Error we haven't read 17 bytes" << endl;
			result=false;
		}
	} else {
		Log::logger->log("SENSORS",DEBUG) << "Sensors Error cab't send teh command over I2C bus" << endl;
		result=false;
	}
	close(file);
	return result;
}
