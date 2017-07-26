#include "httpserver.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include "log.h"
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <http_parser.h>



int HttpServer::response404Callback(HttpRequest * request, HttpResponse * response) {
	Log::logger->log("CNXTCP", DEBUG) << "Entering default callback" <<endl;
	response->setStatusCode(404);
	response->setStatusMessage("NOT FOUND");
	response->send();
	delete response;
	delete request;
}

int HttpServer::responseCorsCallback(HttpRequest * request, HttpResponse * response) {
	Log::logger->log("CNXTCP", DEBUG) << "Entering Cors callback" <<endl;
	response->setStatusCode(204);
	response->setStatusMessage("NO CONTENT");
	response->addCustomHeader("Access-Control-Allow-Origin","*");
	response->addCustomHeader("Access-Control-Allow-Methods","GET,HEAD,PUT,PATCH,POST,DELETE");
	//response->addCustomHeader("Access-Control-Allow-Credentials","*");
	//response->addCustomHeader("Access-Control-Allow-Headers","*");
	response->send();
	delete response;
	delete request;
}


HttpServer::HttpServer(int port, unsigned int size, bool cors):Runnable() {
	this->port=port;
	this->size=size;
	this->count=0;
	this->default_callback=HttpServer::response404Callback;
	this->requests=new std::map<int,HttpRequestParser *>();
	this->handlers=new std::map<int,std::vector<HttpHandler *> *>();
	if (cors) {
		this->add(HTTP_OPTIONS, ".*", HttpServer::responseCorsCallback);
	}
}


void HttpServer::makeSocketNonBlocking(int socket) throw(MakeSocketNonBlockingException){
	int flags, s;
	flags = fcntl (socket, F_GETFL, 0);
	if (flags == -1) {
     throw MakeSocketNonBlockingException("Can't set to F_GETFL");
    }

	flags |= O_NONBLOCK;
	s = fcntl (socket, F_SETFL, flags);
	if (s == -1) {
      throw MakeSocketNonBlockingException("Can't set to F_SETFL");
    }
}

void HttpServer::listen() throw(ConnectionListenException){
	struct sockaddr_in server;
	this->socketfd=::socket(AF_INET,SOCK_STREAM,0);

	bzero(&server,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr=htonl(INADDR_ANY);
	server.sin_port=htons(this->port);
	if (::bind(this->socketfd,(struct sockaddr *)&server,sizeof(server))<0) {
		Log::logger->log("CNXTCP", DEBUG) << "Failed to bind the socket " << this->socketfd << " try to bind to " << this->port<<endl;
		throw ConnectionListenException("Can't bind socket");
	}
	try {
		HttpServer::makeSocketNonBlocking(this->socketfd);
	} catch(MakeSocketNonBlockingException &e) {
		Log::logger->log("CNXTCP", DEBUG) << "Failed to make listen socket non blocking " << this->socketfd << " bind to " << this->port<<endl;
		throw ConnectionListenException("Can't make socket non blocking");
	}
	if (::listen(this->socketfd,this->size)<0) {
		Log::logger->log("CNXTCP", DEBUG) << "Failed to listen on socket " << this->socketfd << " bind to " << this->port<<endl;
		throw ConnectionListenException("Can't listen on socket");
	}
	this->pool=new ConnectionPool();
	this->pool->add(this->socketfd);
	Log::logger->log("CNXTCP", NOTICE) << "Listening on port " << this->port <<endl;
}


void HttpServer::accept() throw(MakeSocketNonBlockingException) {
	Log::logger->log("CNXTCP", DEBUG) << "Wait for a new client" <<endl;
	int clientfd;
	struct sockaddr_in client;
	socklen_t clientsize;
	bzero(&client,sizeof(client));
	clientsize=sizeof(client);
	try {
		do {
			Log::logger->log("CNXTCP",DEBUG) << "ACCEPT LOOP START" <<endl;
			bzero(&client,sizeof(client));
			clientsize=sizeof(client);
			clientfd = ::accept(this->socketfd,(struct sockaddr *)&client,&clientsize);
			Log::logger->log("CNXTCP",DEBUG) << "accept " << clientfd << " " << errno<<endl;
			if (clientfd>=0) {
				HttpServer::makeSocketNonBlocking(clientfd);
				this->pool->add(clientfd);
				Log::logger->log("CNXTCP",DEBUG) << "add client in pool" << clientfd <<endl;
				HttpRequestParser * parser=new HttpRequestParser(clientfd, this);
				Log::logger->log("CNXTCP",DEBUG) << "new parser" << clientfd <<endl;
				this->requests->insert( std::pair<int,HttpRequestParser *>(clientfd,parser) );
				//Log::logger->log("CNXTCP",DEBUG) << "try to init parser" << clientfd <<endl;
				//this->requests[clientfd]->init(clientfd, this);
				this->count++;
				Log::logger->log("CNXTCP", DEBUG) << "New Connection " << this->count <<endl;
			} else {
				Log::logger->log("CNXTCP",DEBUG) << "Accept socket: " << clientfd << " error : " << errno << " " << strerror(errno) <<endl;
			}
			Log::logger->log("CNXTCP",DEBUG) << "ACCEPT LOOP END " << clientfd << " " << errno<<endl;
		} while ((clientfd>0) || (errno==EINTR)) ;
		//((clientfd<0) && ((errno==EINTR) || (errno!=EAGAIN)));
	} catch(ConnectionPoolException &e) {
		Log::logger->log("CNXTCP", ERROR) << "A connection pool error occurs " <<endl;
	}
}

void HttpServer::process(HttpRequest * request, HttpResponse * response) {
	bool found=false;
	unsigned int i=0;
	try {
		std::vector<HttpHandler *> * urls=this->handlers->at(request->getMethod());
		while (!found && (i<urls->size())) {
			found=(*urls)[i]->match(request->getUrl());
			if (!found) i++;
		}
		if (found) {
			(*urls)[i]->process(request, response);
		} else {
			this->default_callback(request, response);
		}
	} catch(std::out_of_range &e) {
			this->default_callback(request, response);		
	}

}

void HttpServer::setDefaultCallback(http_callback callback) {
	this->default_callback=callback;
}

void HttpServer::add(unsigned int method, string url, http_callback callback) {
	HttpHandler * handler=new HttpHandler(url, callback);
	std::vector<HttpHandler *> * urls=NULL;
	try {
		urls=this->handlers->at(method);
	} catch(std::out_of_range &e) {
		urls = new std::vector<HttpHandler *>();
		this->handlers->insert( std::pair<int,std::vector<HttpHandler *> *>(method,urls) );
	}
	urls->push_back(handler);
}


void HttpServer::run() {
	this->listen();
	this->Runnable::start();
	while (this->Runnable::running()) {
		vector<int> * ready=this->pool->poll();
		Log::logger->log("CNXTCP",DEBUG) << "poll size "<<  ready->size() <<endl;
		for (unsigned int i=0; i<ready->size(); i++ ) {
			if (ready->at(i)==this->socketfd) {
				Log::logger->log("CNXTCP",DEBUG) << "Socket is a new connection "<<  ready->at(i) <<endl;
				this->accept();
			} else {
				int readerror=0;
				do {
					readerror=0;
					ssize_t nbb;
	                char buf[512];
	                Log::logger->log("CNXTCP",DEBUG) << "Trying to read in socket "<< ready->at(i)<<endl;
	                nbb = read (ready->at(i),  buf, sizeof buf);
	               	readerror=errno;
	                if (nbb>0) {
	                	Log::logger->log("CNXTCP",DEBUG) << "Socket is a client one with something to read "<<  ready->at(i) << " => " <<nbb << " bytes"<<endl;
	                	bool finished=(*this->requests)[ready->at(i)]->received(buf, nbb);
	                	if (finished) {
	                		Log::logger->log("CNXTCP",DEBUG) << "End receiving request" <<endl;
	                		HttpRequestParser * parser=(*this->requests)[ready->at(i)];
	                		this->requests->erase(ready->at(i));
	                		delete parser;

	                	}
	                }
	                Log::logger->log("CNXTCP",DEBUG) << "Read loop "<< readerror<<endl;
	            } while (readerror==0);
			}
		}
		delete ready;
	}

}