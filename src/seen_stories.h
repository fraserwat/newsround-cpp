#pragma once

#include <memory>
#include <string>

class SeenStories
{
public:
  explicit SeenStories(const std::string &db_path);
  ~SeenStories();

  SeenStories(const SeenStories &) = delete;
  SeenStories &operator=(const SeenStories &) = delete;
  SeenStories(SeenStories &&) = delete;
  SeenStories &operator=(SeenStories &&) = delete;

  [[nodiscard]] bool has_seen(const std::string &url) const;
  void mark_seen(const std::string &url);

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};
