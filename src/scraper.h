#pragma once

#include <string>
#include <vector>
#include "websites.h"

[[nodiscard]] Story website_handler(const Website* site);
[[nodiscard]] std::string format_summarisation_call(const std::vector<Story>& story_texts);
