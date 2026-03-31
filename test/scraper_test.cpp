#include "../src/parser.h"
#include "../src/scraper.h"
#include <gtest/gtest.h>

// Base case, you are given the full URL for the story in the <a> tag's href.
TEST(Scraper, ExtractHTMLFull)
{
  std::string website = "https://example.com";
  std::string html = R"(<span class="title"><a href="https://example.com/story">Example Story</a></span>)";
  std::string link = extract_href(html, website);
  ASSERT_EQ(link, "https://example.com/story");
}

// Case where you are only given the post-website stub within the href and you have to construct a
// full URL from that.
TEST(Scraper, ExtractHTMLStub)
{
  std::string website = "https://example.com";
  std::string html = R"(<span class="title"><a href="/story/example-story">Example Story</a></span>)";
  std::string link = extract_href(html, website);
  ASSERT_EQ(link, "https://example.com/story/example-story");
}

TEST(Scraper, ExtractInnerText)
{
  std::string tag = "a";
  std::string html =
    R"(<a class="title" href="/essential-releases/essential-releases-march-13-2026">Essential Releases, March 13, 2026</a>)";
  std::string inner_text = extract_inner_text(html, tag);
  ASSERT_EQ(inner_text, "Essential Releases, March 13, 2026");
}

// I want the inner text to return one big string for that tag, so we are just getting all
// paragraphs into a single text string view.
TEST(Scraper, ExtractParagraphText)
{
  std::string tag = "p";
  std::string html =
    R"(<h3>Subtitle</h3><p>Paragraph One.</p><p>Paragraph Two.</p><h3>Subtitle Two</h3><p>Paragraph Three.</p>)";
  std::string paragraphs = extract_inner_text(html, tag);
  ASSERT_EQ(paragraphs, "Paragraph One. Paragraph Two. Paragraph Three.");
}

const std::string k_prompt_prefix =
  "Summarise each story. Output exactly one line per story, no other text:\n"
  "<url>|<summary>\n"
  "Rules: ≤50 words per summary; match the tone of the source publication.\n\n";

TEST(FormatSummarisation, EmptyList)
{
  ASSERT_EQ(format_summarisation({}), std::nullopt);
}

TEST(FormatSummarisation, SingleStory)
{
  Website site{ .url = "https://tribunemag.co.uk", .name = "Tribune Magazine", .main_story_selector = "" };
  Story story{ .title = "Test Story", .url = "https://tribunemag.co.uk/story", .website = site, .paragraph_text = "Some paragraph text." };
  auto result = format_summarisation({ story });
  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(*result, k_prompt_prefix +
    "## Tribune Magazine | https://tribunemag.co.uk\n\n"
    "**Test Story** (https://tribunemag.co.uk/story)\n"
    "Some paragraph text.\n");
}

TEST(FormatSummarisation, SingleStoryNoParagraphText)
{
  Website site{ .url = "https://tribunemag.co.uk", .name = "Tribune Magazine", .main_story_selector = "" };
  Story story{ .title = "Test Story", .url = "https://tribunemag.co.uk/story", .website = site, .paragraph_text = "" };
  auto result = format_summarisation({ story });
  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(*result, k_prompt_prefix +
    "## Tribune Magazine | https://tribunemag.co.uk\n\n"
    "**Test Story** (https://tribunemag.co.uk/story)\n"
    "\n");
}

TEST(FormatSummarisation, MultipleStories)
{
  Website site_a{ .url = "https://tribunemag.co.uk", .name = "Tribune Magazine", .main_story_selector = "" };
  Website site_b{ .url = "https://news.ycombinator.com", .name = "Hacker News", .main_story_selector = "" };
  std::vector<Story> stories{
    { .title = "Story One", .url = "https://tribunemag.co.uk/one", .website = site_a, .paragraph_text = "Text one." },
    { .title = "Story Two", .url = "https://news.ycombinator.com/two", .website = site_b, .paragraph_text = "Text two." },
  };
  auto result = format_summarisation(stories);
  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(*result, k_prompt_prefix +
    "## Tribune Magazine | https://tribunemag.co.uk\n\n"
    "**Story One** (https://tribunemag.co.uk/one)\n"
    "Text one.\n"
    "\n---\n\n"
    "## Hacker News | https://news.ycombinator.com\n\n"
    "**Story Two** (https://news.ycombinator.com/two)\n"
    "Text two.\n");
}

TEST(Scraper, ExtractParagraphsWithFormatting)
{
  std::string tag = "p";
  std::string html =
    R"(<h3>Subtitle</h3><p>Paragraph One.</p><h3>Subtitle Two</h3><p>Paragraph Two, <b>with emphasis</b>.</p>)";
  std::string paragraphs = extract_inner_text(html, tag);
  ASSERT_EQ(paragraphs, "Paragraph One. Paragraph Two, with emphasis.");
}
