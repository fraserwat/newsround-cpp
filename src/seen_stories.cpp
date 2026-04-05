#include "seen_stories.h"

#include <filesystem>
#include <iostream>
#include <rocksdb/db.h>
#include <stdexcept>
#include <string_view>

namespace {
std::string_view normalize_url(std::string_view url)
{
  if (url.starts_with("https://")) return url.substr(8);
  if (url.starts_with("http://")) return url.substr(7);
  return url;
}
}// namespace

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

  // Creating an "empty slot" for the db in a raw pointer. rocksdb uses a double pointer in its API
  // which writes to rocksdb::DB** (pointer to a pointer) so it needs a raw pointer to write into.
  // We want to create this raw pointer, and then move ownership of it to a unique_ptr to adhere
  // to the Rule of Zero and avoid any memory management ourselves.
  rocksdb::DB *raw_db = nullptr;
  // Initialise the raw_db.
  rocksdb::Status status = rocksdb::DB::Open(options, db_path, &raw_db);
  if (!status.ok()) throw std::runtime_error("Failed to open RocksDB: " + status.ToString());
  // Transfer ownership of db into a unique_ptr
  impl_->db.reset(raw_db);
}

SeenStories::~SeenStories() = default;

bool SeenStories::has_seen(std::string_view url) const
{
  // Uses bloom filters to quickly check existance. There is some small chance of false positives,
  // but this is fine for our purposes.
  return impl_->db->KeyMayExist(rocksdb::ReadOptions(), normalize_url(url), nullptr);
}

void SeenStories::mark_seen(std::string_view url)
{
  rocksdb::Status status = impl_->db->Put(rocksdb::WriteOptions(), normalize_url(url), "1");
  if (!status.ok()) { std::cerr << "URL " << url << " not written to RocksDB store: " << status.ToString() << '\n'; }
}
