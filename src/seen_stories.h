#pragma once

#include <memory>
#include <string>
#include <string_view>

class SeenStories
{
public:
  explicit SeenStories(const std::string &db_path);
  ~SeenStories();

  SeenStories(const SeenStories &) = delete;
  SeenStories &operator=(const SeenStories &) = delete;
  SeenStories(SeenStories &&) = delete;
  SeenStories &operator=(SeenStories &&) = delete;

  [[nodiscard]] bool has_seen(std::string_view url) const;
  void mark_seen(std::string_view url);

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};
