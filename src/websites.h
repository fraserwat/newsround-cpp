#pragma once

#include <string>

struct Website
{
  std::string url;
  std::string name;
  std::string main_story_selector;
};

struct Story
{
  std::string title;
  std::string url;
  Website website;
  std::string paragraph_text;
};

extern const Website tribune;
extern const Website bandcamp;
extern const Website hackernews;
extern const Website financial_times;
