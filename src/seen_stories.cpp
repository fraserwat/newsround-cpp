#include "seen_stories.h"

#include <filesystem>
#include <rocksdb/db.h>
#include <stdexcept>
#include <string_view>

// Strip http:// or https:// scheme so both variants resolve to the same key.
// static as this is a private helper which does not need to be seen outside of this file.
static std::string normalize_url(const std::string &url)
{
  // constexpr to define at compile time.
  constexpr std::string_view https_prefix = "https://";
  constexpr std::string_view http_prefix = "http://";

  if (url.starts_with(https_prefix)) {
    return url.substr(https_prefix.size());
  }
  if (url.starts_with(http_prefix)) {
    return url.substr(http_prefix.size());
  }
  return url;
}

struct SeenStories::Impl
{
  rocksdb::DB *db = nullptr;
  // TODO(claude): you have the constructor, destructor, copy constructor and copy assignment constructor. We are using C++>11, so where is the move operator and move assignment operator, to satisfy the rule of five?
  // TODO(claude): Expain to me why we are defining all of these memory management methods instead of using the rule of zero. You later use a uniue pointer.
  // TODO(claude): What is impl here? we have SeenStories constructor defined below, why are we defining Impl constructors and destructors?
  Impl() = default;
  ~Impl() { delete db; }
  Impl(const Impl &) = delete;
  Impl &operator=(const Impl &) = delete;
};

SeenStories::SeenStories(const std::string &db_path)
  : impl_(std::make_unique<Impl>())
{
  std::filesystem::create_directories(db_path);

  rocksdb::Options options;
  options.create_if_missing = true;

  rocksdb::Status status = rocksdb::DB::Open(options, db_path, &impl_->db);
  if (!status.ok()) {
    throw std::runtime_error("Failed to open RocksDB: " + status.ToString());
  }
}

SeenStories::~SeenStories() = default;

bool SeenStories::has_seen(const std::string &url) const
{
  std::string key = normalize_url(url);
  std::string value;
  // Return non-"ok" status if Get does not return a value, therefore ultimately returning "false".
  rocksdb::Status status = impl_->db->Get(rocksdb::ReadOptions(), key, &value);
  return status.ok();
}

void SeenStories::mark_seen(const std::string &url)
{
  std::string key = normalize_url(url);
  impl_->db->Put(rocksdb::WriteOptions(), key, "1");
}
