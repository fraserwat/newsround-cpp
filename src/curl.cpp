#include <curl/curl.h>
#include <stdexcept>
#include <string>

#include "curl.h"

Curl::Curl() { init(); };

void Curl::init()
{// NOLINT(readability-convert-member-functions-to-static)
  _ready = (curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK);
}

bool Curl::is_ready() const { return _ready; }

void Curl::clear() { curl_global_cleanup(); }

// // This is only needed by other functions inside the curl API, so making it static.
static size_t write_callback(const char *ptr, size_t size, size_t numbytes, void *data)
{
  // Calculate the total size of the incoming HTML chunk.
  size_t totalBytes = size * numbytes;
  // curl library requires writeback definition to include void* data, but we need to
  // convert in order to append. You cannot convert void* to a non-pointer, so we
  // cast to std::string* before appending char* of size totalBytes.
  static_cast<std::string *>(data)->append(ptr, totalBytes);

  return totalBytes;
}

// The processing of the api response, iteratively writing (with the callback fn) to a string block.
std::string curl_wrapper(const std::string &url)
{
  // Declaring an empty string where the html would be written.
  std::string html;

  CURL *handle = curl_easy_init();
  CURLcode response;
  if (handle) {
    curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(handle,
      CURLOPT_USERAGENT,
      "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 "
      "Safari/537.36");
    curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);

    curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    headers = curl_slist_append(headers, "Accept-Language: en-GB,en;q=0.5");
    curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_callback);
    // Using a pointer to html to avoid expensive copy.
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &html);
    response = curl_easy_perform(handle);
    curl_slist_free_all(headers);
    if (response != CURLE_OK) {
      curl_easy_cleanup(handle);
      throw std::runtime_error(curl_easy_strerror(response));
    }
  }
  curl_easy_cleanup(handle);
  return html;
}
