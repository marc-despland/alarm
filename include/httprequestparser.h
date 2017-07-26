#ifndef _HTTPREQUESTPARSER_H
#define _HTTPREQUESTPARSER_H
#include <string>
#include <map>
#include <http_parser.h>
#include "ihttprequestprocessor.h"

class HttpRequestParser {
public:
	HttpRequestParser(int socket, IHttpRequestProcessor * hrp);
	~HttpRequestParser();
	void init(int socket, IHttpRequestProcessor * hrp);
	bool received(char * buffer, size_t length);
protected:
	http_parser_settings * settings;
	http_parser *parser;
	std::string header;
	IHttpRequestProcessor * hrp;
	HttpRequest * request;
	HttpResponse * response;
	bool finished;

	static int on_url(http_parser* parser, const char *at, size_t length);
	static int on_header_field(http_parser* parser, const char *at, size_t length);
	static int on_header_value(http_parser* parser, const char *at, size_t length);
	static int on_message_complete(http_parser* parser);

};

#endif