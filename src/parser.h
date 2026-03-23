#pragma once

#include <string>
#include <vector>

[[nodiscard]] std::vector<std::string> parse(const std::string &html, const std::string &selector);
[[nodiscard]] std::string extract_href(const std::string &html, const std::string &base_url);
[[nodiscard]] std::string extract_inner_text(const std::string &html, const std::string &tag);
