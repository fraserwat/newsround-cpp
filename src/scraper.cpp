#include <vector>
#include <string>
#include <iostream>
#include "scraper.h"
#include "parser.h"
#include "curl.h"

Story website_handler(const Website* site)
{
  // Get the website's front page.
  std::cout << site->url << '\n';
  std::string front_page = curl_wrapper(site->url);

  // Extract the top stories from the front page using that website's CSS selector.
  std::vector<std::string> top_stories = parse(front_page, site->main_story_selector);
  if (top_stories.empty()) { return Story{}; }

  // TODO: Check stories against key-value store and set the first unread one as today's story.
  std::string chosen_story = top_stories[0];
  std::cout << chosen_story << '\n' << '\n';
  // Get the HTML from today's story.
  // std::string story_html = curl_wrapper

  // Pull all the paragraph text for summarisation.

  // Return Story object.
  return Story{};
}
