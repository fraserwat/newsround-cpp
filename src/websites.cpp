#include "websites.h"

const Website tribune{
    .url = "https://tribunemag.co.uk",
    .name = "Tribune Magazine",
    .main_story_selector = "a.hm-dg__link"
};

const Website bandcamp{
    .url = "https://daily.bandcamp.com",
    .name = "Bandcamp Daily",
    .main_story_selector = "a.title"
};

const Website hackernews{
    .url = "https://news.ycombinator.com",
    .name = "Hacker News",
    .main_story_selector = ".titleline > a"
};

const Website financial_times{
    .url = "https://www.ft.com/world-uk",
    .name = "Financial Times",
    .main_story_selector = ".css-grid__item-top a.js-teaser-heading-link"
};
