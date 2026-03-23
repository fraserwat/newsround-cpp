#include <vector>
#include <string>
#include <iostream>
#include "scraper.h"
#include "parser.h"
#include "curl.h"

Story website_handler(const Website* site)
{
  // Get the website's front page.
  std::string front_page = curl_wrapper(site->url);

  // Extract the top stories from the front page using that website's CSS selector.
  std::vector<std::string> top_stories = parse(front_page, site->main_story_selector);
  if (top_stories.empty()) { return Story{}; }

  // TODO: Check stories against key-value store and set the first unread one as today's story.
  Story chosen_story = {
    .title = extract_inner_text(top_stories[0], "a"),
    .url = extract_href(top_stories[0], site->url),
    .website = *site,
  };
  std::cout << chosen_story.title << '\n';
  std::cout << chosen_story.url << '\n';
  // std::cout << chosen_story.paragraph_text << '\n';
  // Get the HTML from today's story.
  std::string story_html = curl_wrapper(chosen_story.url);

  // Pull all the paragraph text for summarisation.
  chosen_story.paragraph_text = extract_inner_text(story_html, "p");
  std::cout << chosen_story.paragraph_text << '\n' << '\n';

  // Return Story object.
  return Story{};
}
