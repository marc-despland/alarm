#ifndef PTHREADEXCEPTION_H
#define PTHREADEXCEPTION_H

#include <exception>
using namespace std;

class ThreadCreateException: public exception {
		const char* what();
};

#endif