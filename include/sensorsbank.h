#ifndef SENSORSBANK_H
#define SENSORSBANK_H

#include <exception>
#include <string>
#include <map>
#include "curlwrapper.h"
using namespace std;

class PostException: public exception {
		const char* what();
};


class SensorsBank {
public:
	static string ApiUrl;
	static string ApiKey;
	static void sendData(std::map<string,double> sesnors) throw (CurlInitException, PostException);
protected:
	static string toJson(std::map<string,double> sesnors);

};
#endif