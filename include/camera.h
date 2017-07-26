#ifndef CAMERA_H
#define CAMERA_H


#include <exception>
#include <string>
#include <raspicam/raspicam.h>
#include <pthread.h>

using namespace std;

class CameraOpenException: public exception {
		const char* what();
};

class Camera {
	public:
		Camera();
		~Camera();
		void init() throw (CameraOpenException);
		//void release();
		void capture() throw (CameraOpenException);
		unsigned char * toJpeg (unsigned long *mem_size,int quality);
	protected:
		raspicam::RaspiCam camera;
		unsigned char *data;
		bool camera_init;
		static pthread_mutex_t lock;
};



#endif