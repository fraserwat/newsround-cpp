#include "curl.h"
#include "scraper.h"
#include "seen_stories.h"
#include "websites.h"
#include <algorithm>
#include <cstdlib>
#include <vector>
#include <iostream>

int main()
{
  std::vector<const Website *> websites = { &tribune, &bandcamp, &hackernews, &financial_times };
  // Global initialisation.
  Curl curl;
  const char *home = std::getenv("HOME");// NOLINT(concurrency-mt-unsafe)
  SeenStories seen(std::string(home != nullptr ? home : ".") + "/.newsround/seen_stories");
  // Multi-threaded website handling.
  std::vector<Story> todays_stories(websites.size());
  // TODO: Parallelise once the sequential version works.
  std::ranges::transform(
    websites, todays_stories.begin(), [&seen](const Website *site) { return website_handler(site, seen); });

  for (const auto &story : todays_stories) {
    std::cout << story.website.name << ": " << story.title << '\n' << story.url << '\n' << '\n';
  }

  // Format stories for summarisation by LLM and push to LLM API call.
  // std::string llm_response = llm_summarisation(format_summarisation(todays_stories));

  // Send to email construction function.
  // send_email(llm_response&);

  return 0;
}
