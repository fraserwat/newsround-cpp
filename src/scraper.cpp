#include "scraper.h"
#include "curl.h"
#include "parser.h"
#include <algorithm>
#include <optional>
#include <string>
#include <vector>

Story website_handler(const Website *site, SeenStories &seen)
{
  // Get the website's front page.
  std::string front_page = curl_wrapper(site->url);

  // Extract the top stories from the front page using that website's CSS selector.
  std::vector<std::string> top_stories = parse(front_page, site->main_story_selector);
  if (top_stories.empty()) { return Story{}; }

  // Find the first story that hasn't been sent in a previous newsletter.
  auto unseen = std::ranges::find_if(
    top_stories, [&](const std::string &html) { return !seen.has_seen(extract_href(html, site->url)); });
  if (unseen == top_stories.cend()) { return Story{}; }

  Story chosen_story = {
    .title = extract_inner_text(*unseen, "a"),
    .url = extract_href(*unseen, site->url),
    .website = *site,
  };
  seen.mark_seen(chosen_story.url);

  // Get the HTML from today's story.
  std::string story_html = curl_wrapper(chosen_story.url);

  // Pull all the paragraph text for summarisation.
  chosen_story.paragraph_text = extract_inner_text(story_html, "p");

  return chosen_story;
}

std::optional<std::string> format_summarisation(const std::vector<Story> &stories)
{
  if (stories.empty()) { return std::nullopt; }
  std::string out =
    "Summarise each story. Output exactly one line per story, no other text:\n"
    "<url>|<summary>\n"
    "Rules: ≤50 words per summary; match the tone of the source publication.\n\n";
  for (std::size_t i = 0; i < stories.size(); ++i) {
    const Story &story = stories[i];
    out += "## " + story.website.name + " | " + story.website.url + "\n\n";
    out += "**" + story.title + "** (" + story.url + ")\n";
    out += story.paragraph_text + "\n";
    if (i + 1 < stories.size()) { out += "\n---\n\n"; }
  }
  return out;
}

std::string llm_summarisation(const std::string_view /*input*/) { return {}; }
