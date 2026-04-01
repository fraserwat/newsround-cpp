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

  if (url.starts_with(https_prefix)) { return url.substr(https_prefix.size()); }
  if (url.starts_with(http_prefix)) { return url.substr(http_prefix.size()); }
  return url;
}

struct SeenStories::Impl
{
  std::unique_ptr<rocksdb::DB> db;
};

SeenStories::SeenStories(const std::string &db_path) : impl_(std::make_unique<Impl>())
{

  // Create folder for db if it doesn't already exist.
  std::filesystem::create_directories(db_path);

  // Define the config.
  rocksdb::Options options;
  options.create_if_missing = true;

  // Creating an "empty slot" for the db in a raw pointer. rocksdb is a C-style API which writes to
  // rocksdb::DB** (pointer to a pointer) so it needs a raw pointer to write into. We want to
  // create this raw pointer, and then move ownership of it to a unique_ptr to adhere to the
  // Rule of Zero and avoid any memory management ourselves.
  rocksdb::DB *raw_db = nullptr;
  // Initialise the raw_db.
  rocksdb::Status status = rocksdb::DB::Open(options, db_path, &raw_db);
  if (!status.ok()) { throw std::runtime_error("Failed to open RocksDB: " + status.ToString()); }
  // Transfer ownership of db into a unique_ptr
  impl_->db.reset(raw_db);
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
