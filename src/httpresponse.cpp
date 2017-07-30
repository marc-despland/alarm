#include "httpresponse.h"
#include <sstream>
#include <unistd.h>
#include <string.h>
#include "log.h"
#include <sys/select.h>

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

void HttpResponse::setBody(const char *buffer, unsigned long size) {
	this->buffer=(char *) buffer;
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
	int ws=this->write(headers.str().c_str(), headers.str().length());
	if (ws<0) {
		Log::logger->log("HTTPRESPONSE",ERROR) << "Can't write on socket: " << this->socket <<endl;
	} else {
		char * ptr=this->buffer;
		unsigned int length=this->contentLength;
		Log::logger->log("HTTPRESPONSE",DEBUG) << "Starting writing body." << endl;
		while (length>0) {
			Log::logger->log("HTTPRESPONSE",DEBUG) << "We have to write " << length << " bytes" <<endl;
			ws=this->write(ptr, length);
			Log::logger->log("HTTPRESPONSE",DEBUG) << "We have writen " << ws << " bytes" <<endl;
			if (ws<0) {
				Log::logger->log("HTTPRESPONSE",ERROR) << "Can't write on socket: " << this->socket << " Error : " << strerror(errno) <<endl;
				length=0;
			} else {
				length-=ws;
				if (length>0) ptr+=ws;
			}
		}
	}
	close(this->socket);	                		
}

int HttpResponse::write(const char * buffer, unsigned long size) {
	fd_set write_flags; // the flag sets to be used
    struct timeval waitd;          // the max wait time for an event
    int sel;
    waitd.tv_sec = 10;
    FD_ZERO(&write_flags);
    FD_SET(this->socket, &write_flags);
    sel = select(this->socket+1, (fd_set*)0, &write_flags, (fd_set*)0, &waitd);
    if(sel <= 0) {
    	Log::logger->log("HTTPRESPONSE",DEBUG) << "Failed to wait select "<<sel <<endl;
    	return -1;
    }
    if (FD_ISSET(this->socket, &write_flags)) {
    	FD_CLR(socket, &write_flags);
    	int ws=::write(this->socket, buffer, size);
    	return ws;
    } else {
    	Log::logger->log("HTTPRESPONSE",DEBUG) << "Not sure it's possible to come here" <<endl;
    	return -1;
    }

}