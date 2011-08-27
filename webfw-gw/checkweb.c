#include <glib.h>
#include <curl/curl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "header.h"

static size_t
fetch_list_callback(void *ptr, size_t size, size_t nmemb, void *data)
{
  size_t realsize = size * nmemb;
  GString *current_csv = (GString *)data;

  g_string_append(current_csv,ptr);
  return realsize;
}

gboolean fetch_entry(const gchar *url,struct value_struct *connection){
  CURL *curl;
  CURLcode res;
  GString *tmp;
  GString *error =g_string_new("SYSTEM ERROR");
  gchar *getreq;
  gboolean retval = FALSE;
  glong http_error_code=0;

  getreq = g_strdup_printf("%s?src_ip=%s",url,connection->s_src_ip);
  g_debug("getreq == %s\n",getreq);

  tmp = g_string_new("");
  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, getreq);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fetch_list_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) tmp);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "webfw/1.0");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);

    if(
        (0 == (res = curl_easy_perform(curl)))
        && (tmp->len > 0)
        && (FALSE == g_str_has_prefix(tmp->str,"\nSYSTEM ERROR\n"))
        && (CURLE_OK == curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_error_code))
        && (http_error_code < 300)
      ){
                if (!g_string_equal(tmp,error)){
                        g_debug("User Authentiticated with Website as");
                        g_debug("%s",*tmp);
                        connection->username = g_strdup_printf("%s",*tmp);
                        retval = TRUE;
                }
        }
    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  g_string_free(tmp,TRUE);
  g_string_free(error,TRUE);
  g_free(getreq);
  return retval;
}