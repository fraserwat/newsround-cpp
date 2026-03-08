#include <curl/curl.h>
#include <curl/multi.h>
#include <iostream>
#include <string>
#include <vector>

#include "curl.h"

Curl::Curl() { init(); };

// This is the equivalent of a main() function. Libcurl setups up the environment.
// https://curl.se/libcurl/c/curl_global_init.html.
// Then our wrapper is called which processes HTML from each of the URLs in parallel.
// An equivalent cleanup function runs releasing resources acquired by init, and we
// have our html blocks to deal with our own logic. Lightweight as possible.
// cppcheck-suppress functionStatic // TODO: remove once member vars are added
void Curl::init() { // NOLINT(readability-convert-member-functions-to-static)
	curl_global_init(CURL_GLOBAL_DEFAULT);
}

// cppcheck-suppress functionStatic // TODO: remove once member vars are added
void Curl::clear() { // NOLINT(readability-convert-member-functions-to-static)
	curl_global_cleanup();
}

// // This is only needed by other functions inside the curl API, so making it static.
// static size_t WriteCallback(const char* /*ptr*/, size_t size, size_t numbytes, const void* /*data*/) {
// 	// https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
// 	// Calculate the total size of the incoming HTML chunk.
// 	size_t totalBytes = size * numbytes;
// 	return totalBytes;
// }

// For each website, a URL is taken in and HTML is returned.
// cppcheck-suppress constParameterReference // TODO: remove once CurlWrapper is implemented
std::vector<std::string> CurlWrapper(std::vector<std::string>& urls) { // NOLINT(misc-unused-parameters,cppcoreguidelines-avoid-const-or-ref-data-members)

	// https://curl.se/libcurl/c/curl_multi_init.html
	CURLM *multi = curl_multi_init();

	for (const auto& url : urls) { // NOLINT(misc-unused-variables) // TODO: remove once loop body is implemented
		//
		// curl_easy_init
		// curl_easy_setopt(url, WriteCallback, buffer);
		// curl_multi_add_handle
		//
		std::cout << url << '\n';
	}

	// do { curl_multi_perform } while (still_running)
	//
	// for each handle: curl_multi_remove_handle, curl_easy_cleanup

	curl_multi_cleanup(multi);
	// return html
	return {};
}
