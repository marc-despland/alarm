#ifndef IMAGESBANK_H
#define IMAGESBANK_H

#include <exception>
#include <string>
using namespace std;

class CurlInitException: public exception {
		const char* what();
};

class UploadException: public exception {
		const char* what();
};


class ImagesBank {
public:
	static string ApiUrl;
	static string ApiKey;
	static string createCollection();
	static void upload(string collection, string filename, unsigned char * buffer, unsigned long size) throw (CurlInitException, UploadException);

};
#endif