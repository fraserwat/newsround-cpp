#include "scraper.h"

Story website_handler(const Website * /*site*/)
{
  // Get the website's front page.

  // Extract the top stories from the front page using that website's CSS selector.

  // TODO: Check stories against key-value store and set the first unread one as today's story.

  // Get the HTML from today's story.

  // Pull all the paragraph text for summarisation.

  // Return Story object.
  return Story{};
}
