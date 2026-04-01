#pragma once

#include "seen_stories.h"
#include "websites.h"
#include <optional>
#include <string>
#include <string_view>
#include <vector>

[[nodiscard]] Story website_handler(const Website *site, SeenStories &seen);
[[nodiscard]] std::optional<std::string> format_summarisation(const std::vector<Story> &stories);
[[nodiscard]] std::string llm_summarisation(std::string_view input);
