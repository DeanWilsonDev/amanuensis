// NOTE: OrderedMap maintains insertion order.
// This means the JSON output is always the same, making it easier to read and version control
// The drawback is that performing deletions is difficult.

#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "amanuensis/errors.hpp"

namespace Amanuensis {

template <typename TValue> class OrderedMap {
public:
  void Insert(std::string key, TValue value)
  {
    auto existing_entry = key_to_index_.find(key);
    if (existing_entry != key_to_index_.end()) {
      entries_[existing_entry->second].second = std::move(value);
    }
    else {
      key_to_index_[key] = entries_.size();
      entries_.emplace_back(std::move(key), std::move(value));
    }
  }

  bool Contains(const std::string& key) const { return key_to_index_.count(key) > 0; }

  const TValue& Get(const std::string& key) const
  {
    auto found_entry = key_to_index_.find(key);
    if (found_entry == key_to_index_.end()) {
      throw KeyNotFoundError("Key not found: \"" + key + "\"");
    }
    return entries_[found_entry->second].second;
  }

  TValue& Get(const std::string& key)
  {
    auto found_entry = key_to_index_.find(key);
    if (found_entry == key_to_index_.end()) {
      throw KeyNotFoundError("Key not found: \"" + key + "\"");
    }
    return entries_[found_entry->second].second;
  }

  const std::vector<std::pair<std::string, TValue>>& GetEntries() const { return entries_; }

  const TValue* Find(const std::string& key) const
  {
    auto found_entry = key_to_index_.find(key);
    if (found_entry == key_to_index_.end()) {
      return nullptr;
    }
    return &entries_[found_entry->second].second;
  }

  std::size_t Size() const { return entries_.size(); }

private:
  std::unordered_map<std::string, std::size_t> key_to_index_;
  std::vector<std::pair<std::string, TValue>> entries_;
};

} // namespace Amanuensis
