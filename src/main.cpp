#include <vector>
#include "curl.h"
#include "scraper.h"
#include "websites.h"

int main() {
    std::vector<const Website*> websites = {&tribune, &bandcamp, &hackernews, &financial_times};
    // Global initialisation.
    Curl curl;
    // Multi-threaded website handling.
    std::vector<Story> todays_stories;
    todays_stories.reserve(websites.size());
    // TODO: Parallelise once the sequential version works.
    std::transform(websites.cbegin(), websites.cend(), todays_stories.begin(), website_handler);
 
    // Format stories for summarisation by LLM and push to LLM API call.
    // std::string llm_response = llm_summarisation(format_summarisation(todays_stories));

    // Send to email construction function.
    // send_email(llm_response&);

    return 0;
}
