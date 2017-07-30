#ifndef ISTATUS_H
#define ISTATUS_H

#include <string>

class IStatus {
	public:
		virtual std::string jsonStatus()=0;
};

#endif