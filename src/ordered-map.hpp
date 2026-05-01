#pragma once

#include "amanuensis/value.hpp"
#include <unordered_map>
#include <vector>

namespace Amanuensis {

// NOTE: OrderedMap maintains insertion order.
// This means the JSON output is always the same, making it easier to read and version control
// The drawback is that performing deletions is difficult.
class OrderedMap {
public:
  void Insert(std::string key, Value value);
  bool Contains(const std::string& key) const;
  const Value& Get(const std::string& key) const;
  Value& Get(const std::string& key);
  const std::vector<std::pair<std::string, Value>>& GetEntries() const;
  const Value* Find(const std::string& key) const;
  std::size_t Size() const;

private:
  std::unordered_map<std::string, std::size_t> keyToIndex;
  std::vector<std::pair<std::string, Value>> entries;
};
} // namespace Amanuensis
