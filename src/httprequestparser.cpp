#include "httprequestparser.h"
#include "log.h"
#include <string.h>

/*
struct http_parser_settings {
  http_cb      on_message_begin;
  http_data_cb on_url;
  http_data_cb on_status;
  http_data_cb on_header_field;
  http_data_cb on_header_value;
  http_cb      on_headers_complete;
  http_data_cb on_body;
  http_cb      on_message_complete;
  // When on_chunk_header is called, the current chunk length is stored in parser->content_length.
  http_cb      on_chunk_header;
  http_cb      on_chunk_complete;
};

typedef int (*http_data_cb) (http_parser*, const char *at, size_t length);
typedef int (*http_cb) (http_parser*);
*/

HttpRequestParser::HttpRequestParser(int socket, IHttpRequestProcessor * hrp) {
	Log::logger->log("HTTPREQUESTPARSER", DEBUG) << "new "<<endl;
	this->finished=false;
	this->settings=(http_parser_settings *) malloc(sizeof(http_parser_settings));
	http_parser_settings_init(this->settings);
	this->settings->on_url = HttpRequestParser::on_url;
	this->settings->on_header_field = HttpRequestParser::on_header_field;
	this->settings->on_header_value = HttpRequestParser::on_header_value;
	this->settings->on_message_complete= HttpRequestParser::on_message_complete;

	this->parser = (http_parser *) malloc(sizeof(http_parser));
	http_parser_init(this->parser, HTTP_REQUEST);
	this->parser->data = (void *) this;
	this->hrp=hrp;
	this->request=new HttpRequest(socket);
	this->response=new HttpResponse(socket);
}

HttpRequestParser::~HttpRequestParser() {
	free(this->settings);
	free(this->parser);
}

int HttpRequestParser::on_url(http_parser* parser, const char *at, size_t length) {
	HttpRequestParser * me=(HttpRequestParser *) parser->data;
	char * value=(char *) malloc(sizeof(char)*(length+1));
	memcpy( value, at, length );
	value[length]='\0';
	me->request->setUrl(parser->method, value);
	free(value);
	//Log::logger->log("HTTPREQUESTPARSER", DEBUG) << "URL " << value << " length " << length << " Method: " << parser->method<<endl;
	return 0;
}
int HttpRequestParser::on_header_field(http_parser* parser, const char *at, size_t length) {
	Log::logger->log("HTTPREQUESTPARSER", DEBUG) << "on_header_field" <<endl;
	HttpRequestParser * me=(HttpRequestParser *) parser->data;
	char * value=(char *) malloc(sizeof(char)*(length+1));
	memcpy( value, at, length );
	value[length]='\0';
	me->header=value;
	Log::logger->log("HTTPREQUESTPARSER", DEBUG) << "Header Field " << value << " length " << length<<endl;
	free(value);
	return 0;
}
int HttpRequestParser::on_header_value(http_parser* parser, const char *at, size_t length) {
	Log::logger->log("HTTPREQUESTPARSER", DEBUG) << "on_header_value" <<endl;
	HttpRequestParser * me=(HttpRequestParser *) parser->data;
	char * value=(char *) malloc(sizeof(char)*(length+1));
	memcpy( value, at, length );
	value[length]='\0';
	me->request->addHeader(me->header,value);
	Log::logger->log("HTTPREQUESTPARSER", DEBUG) << "Header Value " << value << " length " << length<<endl;
	free(value);
	return 0;
}
int HttpRequestParser::on_message_complete(http_parser* parser){
	Log::logger->log("HTTPREQUESTPARSER", DEBUG) << "on_message_complete" <<endl;
	HttpRequestParser * me=(HttpRequestParser *) parser->data;
	me->finished=true;
	Log::logger->log("HTTPREQUESTPARSER", DEBUG) << "Message complete" <<endl;
	me->hrp->process(me->request,me->response);
	return 0;
}

void HttpRequestParser::init(int socket, IHttpRequestProcessor * hrp) {
	Log::logger->log("HTTPREQUESTPARSER", DEBUG) << "Initialization of HTTPRequestParser socket: " << socket<<endl;
}


bool HttpRequestParser::received(char * buffer, size_t length) {
	http_parser_execute(this->parser, this->settings, buffer, length);
	return this->finished;
}