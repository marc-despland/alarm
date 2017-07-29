#include "sensorsbank.h"
#include <ctime>
#include <curl/curl.h>
#include <sstream>
#include "log.h"
string SensorsBank::ApiKey="";
string SensorsBank::ApiUrl="";

string SensorsBank::toJson(std::map<string,double> sensors) {
	time_t rawtime;
	struct tm * timeinfo;
	char responsedate [80]; 
	time (&rawtime);
	timeinfo = gmtime (&rawtime);
	//2017-07-28T10:35:55.912Z
	strftime (responsedate,80,"\"date\": \"%Y-%m-%dT%H:%M:%S.000Z\"",timeinfo);
	std::stringstream json;
	json << "{" <<endl;
	json << "	"<< responsedate<<","<<endl;
	json << "	\"sensors\": [" << endl;
	for (std::map<string, double>::iterator it=sensors.begin();it!=sensors.end(); it++) {
		if (it!=sensors.begin()) {
			json << ","<<endl;
		}
		json << "		{" <<endl;
		json << "			\"name\": \"" << it->first<< "\"," <<endl;
		json << "			\"value\": " << it->second <<endl;
		json << "		}";
	}
	json << endl << "	]" << endl;
	json << "}" <<endl;
	Log::logger->log("SENSORSBANK",DEBUG) << "Sensors data : " <<endl<< json.str() <<endl;
	return json.str();
}

void SensorsBank::sendData(std::map<string,double> sensors) throw (CurlInitException, PostException) {
	CURL *curl;
	CURLcode res;
	struct curl_slist *list = NULL;

	curl = curl_easy_init();
	if(curl) {
		string apikey="ApiKey: "+SensorsBank::ApiKey;
		list = curl_slist_append(list, apikey.c_str());
		list = curl_slist_append(list, "Content-Type: application/json");
		list = curl_slist_append(list, "Expect:");
		string url=SensorsBank::ApiUrl+"/sensors";
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		string data=SensorsBank::toJson(sensors);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.length());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
		res = curl_easy_perform(curl);
		if(res != CURLE_OK) {
			Log::logger->log("SENSORSBANK",ERROR) << "Failed to send sensors data " << curl_easy_strerror(res) <<endl;
			curl_easy_cleanup(curl);
			throw PostException();
		}
		Log::logger->log("SENSORSBANK",DEBUG) << "Success" <<endl;
		curl_easy_cleanup(curl);
	} else {
		Log::logger->log("SENSORSBANK",ERROR) << "Failed to init curl library " <<endl;
		throw CurlInitException();
	}
}
