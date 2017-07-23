#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
// gcc multipartpost.c -o multipart -lstdc++ -lcurl
int main(void)
{
  CURL *curl;
  CURLcode res;
  struct curl_httppost *formpost=NULL;
  struct curl_httppost *lastptr=NULL;
  struct curl_slist *headerlist=NULL;
  static const char buf[] = "Expect:";
 

  static const char *postthis="moo mooo moo moo";
 

curl_formadd(&formpost,
          &lastptr,
          CURLFORM_COPYNAME, "send",
          CURLFORM_BUFFER, "nowy.jpg",
          CURLFORM_BUFFERPTR, postthis,
          CURLFORM_BUFFERLENGTH, strlen(postthis),
          CURLFORM_END);

  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.1.46:8080");
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform(curl);
    /* Check for errors */ 
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
 
    /* always cleanup */ 
    curl_easy_cleanup(curl);
  }
  return 0;
}
