#include "httphandler.h"
#include "log.h"


HttpHandler::HttpHandler(string url, http_callback callback) {
	this->regex_url=regex(url);
	this->callback=callback;
}


int HttpHandler::process(HttpRequest * request,HttpResponse * response) {
	return this->callback(request, response);
}

bool HttpHandler::match(string url) {
	bool result=std::regex_match (url,this->regex_url);
	Log::logger->log("HTTHANDLER", DEBUG) << "Match url  " <<url<<" ? "<< result<<endl;
	return std::regex_match (url,this->regex_url);
}