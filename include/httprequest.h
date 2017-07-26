#ifndef _HTTPREQUEST_H
#define _HTTPREQUEST_H
#include <string>
#include <map>
#include <http_parser.h>
using namespace std;

class HttpRequest {
public:
	HttpRequest(int socket);
	int getSocket();
	void addHeader(string name, string value);
	void setUrl(unsigned int method,string value);
	unsigned int getMethod();
	string getUrl();
	string getHeader(string name)  throw(std::out_of_range);
protected:
	std::map<std::string,std::string> headers;
	std::string url;
	int socket;
	unsigned int method;

};

#endif