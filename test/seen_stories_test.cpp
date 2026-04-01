#include "seen_stories.h"
#include <filesystem>
#include <gtest/gtest.h>

namespace fs = std::filesystem;

class SeenStoriesTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    db_path_ = fs::temp_directory_path() / "seen_stories_test_db";
    fs::remove_all(db_path_);
  }

  void TearDown() override { fs::remove_all(db_path_); }

  fs::path db_path_;
};

// --- Database initialisation ---

TEST_F(SeenStoriesTest, CreatesNewDatabaseIfNotExists)
{
  EXPECT_FALSE(fs::exists(db_path_));
  SeenStories store(db_path_.string());
  EXPECT_TRUE(fs::exists(db_path_));
}

TEST_F(SeenStoriesTest, EmptyDatabaseHasSeenNothing)
{
  SeenStories store(db_path_.string());
  EXPECT_FALSE(store.has_seen("https://example.com/story"));
}

// --- Core behaviour ---

TEST_F(SeenStoriesTest, UnseenUrlReturnsFalse)
{
  SeenStories store(db_path_.string());
  EXPECT_FALSE(store.has_seen("https://example.com/story-1"));
}

TEST_F(SeenStoriesTest, MarkSeenThenReturnTrue)
{
  SeenStories store(db_path_.string());
  store.mark_seen("https://example.com/story-1");
  EXPECT_TRUE(store.has_seen("https://example.com/story-1"));
}

TEST_F(SeenStoriesTest, MarkingTwiceDoesNotCrash)
{
  SeenStories store(db_path_.string());
  store.mark_seen("https://example.com/story-1");
  store.mark_seen("https://example.com/story-1");
  EXPECT_TRUE(store.has_seen("https://example.com/story-1"));
}

// --- Persistence ---

TEST_F(SeenStoriesTest, MarkedUrlPersistedAcrossInstances)
{
  {
    SeenStories store(db_path_.string());
    store.mark_seen("https://example.com/story-1");
  }
  SeenStories store(db_path_.string());
  EXPECT_TRUE(store.has_seen("https://example.com/story-1"));
}

// --- Edge cases ---

TEST_F(SeenStoriesTest, EmptyStringUrlDoesNotCrash)
{
  SeenStories store(db_path_.string());
  store.mark_seen("");
  EXPECT_TRUE(store.has_seen(""));
}

TEST_F(SeenStoriesTest, TrailingSlashTreatedAsDistinctUrl)
{
  SeenStories store(db_path_.string());
  store.mark_seen("https://example.com/story");
  EXPECT_FALSE(store.has_seen("https://example.com/story/"));
}

TEST_F(SeenStoriesTest, HttpAndHttpsTreatedAsSameUrl)
{
  SeenStories store(db_path_.string());
  store.mark_seen("https://example.com/story");
  EXPECT_TRUE(store.has_seen("http://example.com/story"));
}
