#include "../src/parser.h"
#include <gtest/gtest.h>

// Assumed API: std::vector<std::string> parse(const std::string& html, const std::string& selector);
// Selector formats: "tag", ".class", "tag.class", ".outer .inner" (descendant), ".outer > .inner" (direct child)
// Returns: inner text content of each matching element

TEST(Parser, BasicTests)
{
  // Test for <p>
  {
    std::string html = "<p>Hello world</p>";
    auto result = parse(html, "p");
    ASSERT_EQ(result.size(), 1U);
    EXPECT_EQ(result[0], "Hello world");
  }

  // Test for the class .test
  {
    std::string html = "<div class=\"test\">Found it</div>";
    auto result = parse(html, ".test");
    ASSERT_EQ(result.size(), 1U);
    EXPECT_EQ(result[0], "Found it");
  }

  // Test for an <a> tag with the class .test-link
  {
    std::string html = R"(<a class="test-link" href="/url">Click here</a>)";
    auto result = parse(html, "a.test-link");
    ASSERT_EQ(result.size(), 1U);
    EXPECT_EQ(result[0], "Click here");
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
    EXPECT_EQ(result[0], "First");
    EXPECT_EQ(result[1], "Second");
    EXPECT_EQ(result[2], "Third");
  }

  // Test for a list of items with class .test-class
  {
    std::string html = R"(<div class="test-class">One</div><span class="test-class">Two</span>)";
    auto result = parse(html, ".test-class");
    ASSERT_EQ(result.size(), 2U);
    EXPECT_EQ(result[0], "One");
    EXPECT_EQ(result[1], "Two");
  }

  // Test for multiple li.test-class, when some li != .test-class
  {
    std::string html = "<li class=\"test-class\">Match</li><li>No match</li><li class=\"test-class\">Match 2</li>";
    auto result = parse(html, "li.test-class");
    ASSERT_EQ(result.size(), 2U);
    EXPECT_EQ(result[0], "Match");
    EXPECT_EQ(result[1], "Match 2");
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
    EXPECT_EQ(result[0], "Match");
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
    EXPECT_EQ(result[0], "Direct child");
  }
}
