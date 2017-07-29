#ifndef HTTPCONTEXT_H
#define HTTPCONTEXT_H

#include "httprequest.h"
#include "httpresponse.h"

class HttpContext {
	public:
		HttpContext(HttpRequest * request, HttpResponse * response);
		~HttpContext();
		HttpRequest * request;
		HttpResponse * response;
};

#endif
