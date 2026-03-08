#include <curl/curl.h>
#include <iostream>
#include <string>
#include <stdexcept>

#include "curl.h"

Curl::Curl() { init(); };

void Curl::init() { // NOLINT(readability-convert-member-functions-to-static)
	_ready = (curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK);
}

bool Curl::is_ready() const {
	return _ready;
}

void Curl::clear() {
	curl_global_cleanup();
}

// // This is only needed by other functions inside the curl API, so making it static.
static size_t write_callback(const char* ptr, size_t size, size_t numbytes, void* data) {
	// Calculate the total size of the incoming HTML chunk.
	size_t totalBytes = size * numbytes;
	// curl library requires writeback definition to include void* data, but we need to
	// convert in order to append. You cannot convert void* to a non-pointer, so we
	// cast to std::string* before appending char* of size totalBytes.
	static_cast<std::string*>(data)->append(ptr, totalBytes);

	return totalBytes;
}

// The processing of the api response, iteratively writing (with the callback fn) to a string block.
std::string curl_wrapper(const std::string& url) {
	// Declaring an empty string where the html would be written.
	std::string html;

	CURL* handle = curl_easy_init();
	CURLcode response;
	if (handle) {
		curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
		curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_callback);
		// Using a pointer to html to avoid expensive copy.
		curl_easy_setopt(handle, CURLOPT_WRITEDATA, &html);
		response = curl_easy_perform(handle);
		if (response != CURLE_OK) {
			curl_easy_cleanup(handle);
			throw std::runtime_error(curl_easy_strerror(response));
		}
	}
	curl_easy_cleanup(handle);
	return html;
}
