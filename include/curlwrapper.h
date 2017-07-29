#ifndef CURLWRAPPER_H
#define CURLWRAPPER_H
#include <exception>
using namespace std;
class CurlInitException: public exception {
		const char* what();
};

#endif