#include "httpresponse.h"
#include <sstream>
#include <unistd.h>
#include "log.h"


//144

HttpResponse::HttpResponse(int socket) {
	this->socket=socket;
	this->contentType="text/plain";
	this->statusCode=200;
	this->statusMessage="OK";
	this->contentLength=0;
	this->buffer=NULL;
}

HttpResponse::~HttpResponse() {
}


void HttpResponse::setContentType(string value) {
	this->contentType=value;
}
void HttpResponse::setStatusCode(unsigned int value) {
	this->statusCode=value;
}
void HttpResponse::setStatusMessage(string message) {
	this->statusMessage=message;
}
void HttpResponse::addCustomHeader(string name, string value) {
	this->headers[name]=value;
}

void HttpResponse::setBody(char *buffer, unsigned long size) {
	this->buffer=buffer;
	this->contentLength=size;
}

void HttpResponse::send() {
	time_t rawtime;
	struct tm * timeinfo;
	char responsedate [80]; 
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	strftime (responsedate,80,"Date: %a, %d %b %Y %X %Z",timeinfo);

	std::stringstream headers;
	headers << "HTTP/1.1 "<<this->statusCode<< " " <<this->statusMessage<<"\r\n";
	headers << "Content-Type: "<<this->contentType<<"\r\n";
	headers << "Content-Length: "<<this->contentLength<<"\r\n";
	headers << responsedate<<"\r\n";
	for (std::map<std::string,std::string>::iterator it=this->headers.begin(); it!=this->headers.end(); ++it) {
    	headers << it->first << ": " << it->second << "\r\n";
    }
	headers << "\r\n";
	int ws=write(this->socket, headers.str().c_str(), headers.str().length());
	if (ws<0) {
		Log::logger->log("HTTPRESPONSE",ERROR) << "Can't write on socket: " << this->socket <<endl;
	} else {
		char * ptr=this->buffer;
		unsigned int length=this->contentLength;
		while (length>0) {
			ws=write(this->socket, ptr, length);
			if (ws<0) {
				Log::logger->log("HTTPRESPONSE",ERROR) << "Can't write on socket: " << this->socket <<endl;
				length=0;
			} else {
				length-=ws;
				if (length>0) ptr+=ws;
			}
		}
	}
	close(this->socket);	                		
}