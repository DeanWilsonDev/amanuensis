#include "ordered-map.hpp"

namespace Amanuensis {

void OrderedMap::Insert(std::string key, Value value)
{
  auto existingEntry = this->keyToIndex.find(key);
  if (existingEntry != this->keyToIndex.end()) {
    this->entries[existingEntry->second].second = std::move(value);
  }
  else {
    this->keyToIndex[key] = this->entries.size();
    this->entries.emplace_back(std::move(key), std::move(value));
  }
}

bool OrderedMap::Contains(const std::string& key) const
{
  return this->keyToIndex.count(key) > 0;
}

const Value& OrderedMap::Get(const std::string& key) const
{
  auto foundEntry = this->keyToIndex.find(key);
  if (foundEntry == this->keyToIndex.end()) {
    throw KeyNotFoundError("Key not found: \"" + key + "\"");
  }
  return this->entries[foundEntry->second].second;
}

Value& OrderedMap::Get(const std::string& key)
{
  auto foundEntry = this->keyToIndex.find(key);
  if (foundEntry == this->keyToIndex.end()) {
    throw KeyNotFoundError("Key not found: \"" + key + "\"");
  }
  return this->entries[foundEntry->second].second;
}

const std::vector<std::pair<std::string, Value>>& OrderedMap::GetEntries() const
{
  return this->entries;
}

const Value* OrderedMap::Find(const std::string& key) const
{
  auto foundEntry = this->keyToIndex.find(key);
  if (foundEntry == this->keyToIndex.end()) {
    return nullptr;
  }
  return &this->entries[foundEntry->second].second;
}

std::size_t OrderedMap::Size() const
{
  return this->entries.size();
}

} // namespace Amanuensis
