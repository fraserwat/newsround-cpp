#include "../src/parser.h"
#include <gtest/gtest.h>

// Assumed API: std::vector<std::string> parse(const std::string& html, const std::string& selector);
// Selector formats: "tag", ".class", "tag.class", ".outer .inner" (descendant), ".outer > .inner" (direct child)
// Returns: full element HTML (opening tag + inner text + closing tag) of each matching element

TEST(Parser, BasicTests)
{
  // Test for <p>
  {
    std::string html = "<p>Hello world</p>";
    auto result = parse(html, "p");
    ASSERT_EQ(result.size(), 1U);
    EXPECT_EQ(result[0], "<p>Hello world</p>");
  }

  // Test for the class .test
  {
    std::string html = "<div class=\"test\">Found it</div>";
    auto result = parse(html, ".test");
    ASSERT_EQ(result.size(), 1U);
    EXPECT_EQ(result[0], "<div class=\"test\">Found it</div>");
  }

  // Test for an <a> tag with the class .test-link — href must be present in the result
  {
    std::string html = R"(<a class="test-link" href="/url">Click here</a>)";
    auto result = parse(html, "a.test-link");
    ASSERT_EQ(result.size(), 1U);
    EXPECT_EQ(result[0], R"(<a class="test-link" href="/url">Click here</a>)");
  }

  // Test for no matches
  {
    std::string html = "<div>No paragraphs here</div>";
    auto result = parse(html, "p");
    EXPECT_TRUE(result.empty());
  }
}

TEST(Parser, MultipleReturns)
{
  // Test for a list of <p> tags.
  {
    std::string html = "<p>First</p><p>Second</p><p>Third</p>";
    auto result = parse(html, "p");
    ASSERT_EQ(result.size(), 3U);
    EXPECT_EQ(result[0], "<p>First</p>");
    EXPECT_EQ(result[1], "<p>Second</p>");
    EXPECT_EQ(result[2], "<p>Third</p>");
  }

  // Test for a list of items with class .test-class
  {
    std::string html = R"(<div class="test-class">One</div><span class="test-class">Two</span>)";
    auto result = parse(html, ".test-class");
    ASSERT_EQ(result.size(), 2U);
    EXPECT_EQ(result[0], R"(<div class="test-class">One</div>)");
    EXPECT_EQ(result[1], R"(<span class="test-class">Two</span>)");
  }

  // Test for multiple li.test-class, when some li != .test-class
  {
    std::string html = R"(<li class="test-class">Match</li><li>No match</li><li class="test-class">Match 2</li>)";
    auto result = parse(html, "li.test-class");
    ASSERT_EQ(result.size(), 2U);
    EXPECT_EQ(result[0], R"(<li class="test-class">Match</li>)");
    EXPECT_EQ(result[1], R"(<li class="test-class">Match 2</li>)");
  }
}

TEST(Parser, AdvancedTesting)
{
  // Test for ".outer-class .inner-class" descendants, missing .inner-class instances which
  // are not descendants of .outer-class.
  {
    std::string html =
      "<div class=\"outer-class\">"
      "<p class=\"inner-class\">Match</p>"
      "</div>"
      "<p class=\"inner-class\">No match</p>";
    auto result = parse(html, ".outer-class .inner-class");
    ASSERT_EQ(result.size(), 1U);
    EXPECT_EQ(result[0], "<p class=\"inner-class\">Match</p>");
  }

  // Test for an .inner-class *direct child* of .outer-class, but miss non-direct children.
  {
    std::string html =
      "<div class=\"outer-class\">"
      "<p class=\"inner-class\">Direct child</p>"
      "<div><p class=\"inner-class\">Not direct</p></div>"
      "</div>";
    auto result = parse(html, ".outer-class > .inner-class");
    ASSERT_EQ(result.size(), 1U);
    EXPECT_EQ(result[0], "<p class=\"inner-class\">Direct child</p>");
  }

  // Test for an .inner-class descendant, when the class also has .junk-class before the class we
  // want, and .another-junk-class after it.
  {
    std::string html =
      "<div class=\"outer-class\">"
      "<p class=\"junk-class inner-class another-junk-class\">Match</p>"
      "</div>"
      "<p class=\"inner-class\">No match</p>";
    auto result = parse(html, ".outer-class .inner-class");
    ASSERT_EQ(result.size(), 1U);
    EXPECT_EQ(result[0], "<p class=\"junk-class inner-class another-junk-class\">Match</p>");
  }

  // Test for an <a> tag with href — simulate a real news link selector like ".titleline > a"
  {
    std::string html =
      R"(<span class="titleline"><a href="https://example.com/story">Example Story</a></span>)";
    auto result = parse(html, ".titleline > a");
    ASSERT_EQ(result.size(), 1U);
    EXPECT_EQ(result[0], R"(<a href="https://example.com/story">Example Story</a>)");
  }
}
