#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H


using namespace std;
#include <string>
#include <map>

class HttpResponse {
	public:
		HttpResponse(int socket);
		~HttpResponse();
		void setContentType(string value);
		void setStatusCode(unsigned int value);
		void setStatusMessage(string message);
		void addCustomHeader(string name, string value);
		void setBody(char *buffer, unsigned long size);
		void send();
	protected:
		string contentType;
		unsigned int statusCode;
		string statusMessage;
		std::map<std::string,std::string> headers;
		unsigned long contentLength;
		char * buffer;
		int socket;
};

#endif