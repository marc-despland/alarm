#include "imagesbank.h"
#include <ctime>
#include <curl/curl.h>

#include "log.h"
string ImagesBank::ApiKey="";
string ImagesBank::ApiUrl="";



string ImagesBank::createCollection() {
	time_t now;
    time(&now);
    char buf[sizeof "2011-10-08T07:07:09Z"];
    strftime(buf, sizeof buf, "%FT%TZ", gmtime(&now));
    return buf;
}


void ImagesBank::upload(string collection, string filename, unsigned char * buffer, unsigned long size) throw (CurlInitException, UploadException) {
	CURL *curl;
	CURLcode res;
	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;
	struct curl_slist *list = NULL;
	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "send",
		CURLFORM_BUFFER, filename.c_str(),
		CURLFORM_BUFFERPTR, buffer,
		CURLFORM_BUFFERLENGTH, size,
		CURLFORM_END);

	curl = curl_easy_init();
	if(curl) {
		string apikey="ApiKey: "+ImagesBank::ApiKey;
		list = curl_slist_append(list, apikey.c_str());
		string url=ImagesBank::ApiUrl+"/collections/"+collection+"/images";
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
		res = curl_easy_perform(curl);
		if(res != CURLE_OK) {
			Log::logger->log("IMAGESBANK",ERROR) << "Failed to send image " << curl_easy_strerror(res) <<endl;
			throw UploadException();
		}

		curl_easy_cleanup(curl);
	} else {
		Log::logger->log("IMAGESBANK",ERROR) << "Failed to init curl library " <<endl;
		throw CurlInitException();
	}
}
