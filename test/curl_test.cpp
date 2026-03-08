#include <gtest/gtest.h>
#include "curl.h"

TEST(CurlLibrary, CheckSetup) {
  Curl curl;
  EXPECT_TRUE(curl.is_ready());
  curl.clear();
}

TEST(CurlLibrary, CheckHTMLErr) {
  Curl curl;
  // Casting as void to avoid triggering the nodiscard warning.
  EXPECT_THROW(static_cast<void>(curl_wrapper("not-a-valid-url")), std::runtime_error);
  curl.clear();
}

TEST(CurlLibrary, CheckHTML) {
  Curl curl;
  std::string html = curl_wrapper("https://example.com");
  EXPECT_FALSE(html.empty());
  EXPECT_NE(html.find("<html"), std::string::npos);
  curl.clear();
}
