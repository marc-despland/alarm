#ifndef IHTTPREQUESTPROCESSOR_H
#define IHTTPREQUESTPROCESSOR_H

#include "httprequest.h"
#include "httpresponse.h"

class IHttpRequestProcessor {
	public:
		virtual void process(HttpRequest * request, HttpResponse * response)=0;
};

#endif