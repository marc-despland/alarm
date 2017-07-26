#ifndef _HTTPSERVER_H
#define _HTTPSERVER_H

#include <string>
using namespace std;
#include "runnable.h"
#include "connectionpool.h"
#include <map>
#include <vector>
#include "httprequestparser.h"
#include "httphandler.h"
#include "ihttprequestprocessor.h"

class MakeSocketNonBlockingException: public exception {
	public:
		MakeSocketNonBlockingException(string message):exception() {
			this->message=message;
		}
		~MakeSocketNonBlockingException() throw (){};
		friend std::ostream & operator<<(std::ostream &os, const MakeSocketNonBlockingException& e) {
			return os << e.message;
		}
	protected:
		string message;
};

class ConnectionListenException: public exception {
	public:
		ConnectionListenException(string message):exception() {
			this->message=message;
		}
		~ConnectionListenException() throw (){};
		friend std::ostream & operator<<(std::ostream &os, const ConnectionListenException& e) {
			return os << e.message;
		}
	protected:
		string message;
};
 
class ConnectionAcceptException: public exception {
	public:
		ConnectionAcceptException(string message):exception() {
			this->message=message;
		}
		~ConnectionAcceptException() throw (){};
		friend std::ostream & operator<<(std::ostream &os, const ConnectionAcceptException& e) {
			return os << e.message;
		}
	protected:
		string message;
};
class CantConnectException: public exception {
	public:
		CantConnectException(string message):exception() {
			this->message=message;
		}
		~CantConnectException() throw (){};
		friend std::ostream & operator<<(std::ostream &os, const CantConnectException& e) {
			return os << e.message;
		}
	protected:
		string message;
};


class HttpServer:Runnable, IHttpRequestProcessor {

public:
	HttpServer(int port, unsigned int size, bool cors);
	void listen() throw (ConnectionListenException);
	void accept() throw (MakeSocketNonBlockingException);
	void run();
	void process(HttpRequest * request, HttpResponse * response);
	void add(unsigned int method, string url, http_callback callback);
	void setDefaultCallback(http_callback callback);


protected:
	static void makeSocketNonBlocking(int socket) throw(MakeSocketNonBlockingException);
	int socketfd;
	int port;
	unsigned int size;	
	ConnectionPool * pool;
	unsigned int count;	
	std::map<int,HttpRequestParser *> * requests;
	std::map<int,std::vector<HttpHandler *> *> * handlers;
	http_callback default_callback;
	static int response404Callback(HttpRequest * request, HttpResponse * response);
	static int responseCorsCallback(HttpRequest * request, HttpResponse * response);

};

#endif