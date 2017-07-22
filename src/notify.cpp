#include "notify.h"
#include "log.h"
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <curl/curl.h>

string Notify::ApiKey="";
string Notify::ApiUrl="";

void Notify::notify(int message) {
	int * type=new int();
	*type=message;
	pthread_t notifier_thread;
	if(pthread_create(&notifier_thread, NULL, Notify::send, type)) {
		Log::logger->log("NOTIFY",ERROR) << "Can't create notifier thread " << strerror (errno) << endl;
		return;
	}
	if(pthread_detach(notifier_thread)) {
		Log::logger->log("NOTIFY",ERROR) << "Can't detach notifier thread " << strerror (errno) << endl;
	}
}


void *Notify::send(void *message) {
	Log::logger->log("NOTIFY",DEBUG) << "Notifier thread started " << endl;
	int * type=(int *) message;
	CURL *curl;
	CURLcode res;
	struct curl_slist *list = NULL;
	
	curl = curl_easy_init();
	if(curl) {
		string apikey="apikey: "+Notify::ApiKey;
		list = curl_slist_append(list, apikey.c_str());
		curl_easy_setopt(curl, CURLOPT_URL, Notify::ApiUrl.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

		res = curl_easy_perform(curl);
		if(res != CURLE_OK) {
			Log::logger->log("NOTIFY",ERROR) << "Failed to send notification " << curl_easy_strerror(res) <<endl;
		}
		curl_easy_cleanup(curl);
	}
	delete type;
}
