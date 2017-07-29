#include "httpcontext.h"

HttpContext::HttpContext(HttpRequest * request, HttpResponse * response) {
	this->request=request;
	this->response=response;
}
HttpContext::~HttpContext() {
	delete this->request;
	delete this->response;
}
