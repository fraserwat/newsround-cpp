#pragma once

#include <string>
#include <vector>

[[nodiscard]] std::vector<std::string> parse(const std::string &html, const std::string &selector);
