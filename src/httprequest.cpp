#include "httprequest.h"
#include "log.h"
#include <string.h>


HttpRequest::HttpRequest(int socket) {
	Log::logger->log("HTTPREQUEST", DEBUG) << "Create a new HttpRequest socket: " << socket<<endl;
	this->socket=socket;
}

int HttpRequest::getSocket() {
	return this->socket;
}

void HttpRequest::addHeader(string name, string value) {
	this->headers[name]=value;
}
void HttpRequest::setUrl(unsigned int method, string value) {
	this->url=value;
	this->method=method;
}

unsigned int HttpRequest::getMethod() {
	return this->method;
}
string HttpRequest::getUrl() {
	return this->url;
}
string HttpRequest::getHeader(string name) throw(std::out_of_range) {
	return this->headers.at(name);
}
