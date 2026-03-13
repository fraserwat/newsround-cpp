#include "curl.h"
#include "scraper.h"
#include "websites.h"
#include <algorithm>
#include <vector>

int main()
{
  std::vector<const Website *> websites = { &tribune, &bandcamp, &hackernews, &financial_times };
  // Global initialisation.
  Curl curl;
  // Multi-threaded website handling.
  std::vector<Story> todays_stories;
  todays_stories.reserve(websites.size());
  // Putting empty stories in for std::transform to overwrite.
  for (const auto &_ : websites) {
    todays_stories.push_back(Story{});
  }
  // TODO: Parallelise once the sequential version works.
  std::transform(websites.cbegin(), websites.cend(), todays_stories.begin(), website_handler);

  // Format stories for summarisation by LLM and push to LLM API call.
  // std::string llm_response = llm_summarisation(format_summarisation(todays_stories));

  // Send to email construction function.
  // send_email(llm_response&);

  return 0;
}
