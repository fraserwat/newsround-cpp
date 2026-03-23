#include "../src/parser.h"
#include <gtest/gtest.h>

// Base case, you are given the full URL for the story in the <a> tag's href.
TEST(Scraper, ExtractHTMLFull) {
    std::string website = "https://example.com";
    std::string html =
      R"(<span class="title"><a href="https://example.com/story">Example Story</a></span>)";
    std::string link = extract_href(html, website);
    ASSERT_EQ(link, "https://example.com/story");
}

// Case where you are only given the post-website stub within the href and you have to construct a
// full URL from that.
TEST(Scraper, ExtractHTMLStub) {
    std::string website = "https://example.com";
    std::string html =
      R"(<span class="title"><a href="/story/example-story">Example Story</a></span>)";
    std::string link = extract_href(html, website);
    ASSERT_EQ(link, "https://example.com/story/example-story");
}

TEST(Scraper, ExtractInnerText) {
    std::string tag = "a";
    std::string html = R"(<a class="title" href="/essential-releases/essential-releases-march-13-2026">Essential Releases, March 13, 2026</a>)";
    std::string inner_text = extract_inner_text(html, tag);
    ASSERT_EQ(inner_text, "Essential Releases, March 13, 2026");
}

// I want the inner text to return one big string for that tag, so we are just getting all
// paragraphs into a single text string view.
TEST(Scraper, ExtractParagraphText) {
    std::string tag = "p";
    std::string html = R"(<h3>Subtitle</h3><p>Paragraph One.</p><p>Paragraph Two.</p><h3>Subtitle Two</h3><p>Paragraph Three.</p>)";
    std::string paragraphs = extract_inner_text(html, tag);
    ASSERT_EQ(paragraphs, "Paragraph One. Paragraph Two. Paragraph Three.");
}

TEST(Scraper, ExtractParagraphsWithFormatting) {
    std::string tag = "p";
    std::string html = R"(<h3>Subtitle</h3><p>Paragraph One.</p><h3>Subtitle Two</h3><p>Paragraph Two, <b>with emphasis</b>.</p>)";
    std::string paragraphs = extract_inner_text(html, tag);
    ASSERT_EQ(paragraphs, "Paragraph One. Paragraph Two, with emphasis.");
}
