#ifndef HTTPHANDLER_H
#define HTTPHANDLER_H

#include "httprequest.h"
#include "httpresponse.h"

using namespace std;

#include <string>
#include <regex>

typedef int (*http_callback) (HttpRequest * req, HttpResponse * res);


class HttpHandler{
	public:
		HttpHandler(string url, http_callback callback);
		bool match(string url);
		int process(HttpRequest * req,HttpResponse * res);
	protected:
		std::regex regex_url;
		http_callback callback;
};

#endif