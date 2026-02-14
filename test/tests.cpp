#include <gtest/gtest.h>

TEST(HelloTest, BasicAssertions) {
  // Expect two strings to not be equal
  EXPECT_STRNE("hello", "world");
  // Expect equality
  EXPECT_EQ(1, 2-1);
}
