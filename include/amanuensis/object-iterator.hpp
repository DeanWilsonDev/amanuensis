#pragma once
#include "amanuensis/value.hpp"
#include <string>
#include <vector>

namespace Amanuensis {

class ObjectIterator {
public:
  using UnderlyingIterator = std::vector<std::pair<std::string, Value>>::const_iterator;

  ObjectIterator() = default;
  explicit ObjectIterator(UnderlyingIterator iterator) : iterator_(iterator) {}

  const std::pair<std::string, Value>& operator*() const { return *iterator_; }
  const std::pair<std::string, Value>* operator->() const { return &(*iterator_); }

  ObjectIterator& operator++() { ++iterator_; return *this; }
  ObjectIterator operator++(int) { ObjectIterator previous = *this; ++iterator_; return previous; }

  bool operator==(const ObjectIterator& other) const { return iterator_ == other.iterator_; }
  bool operator!=(const ObjectIterator& other) const { return iterator_ != other.iterator_; }

private:
  UnderlyingIterator iterator_;
};

} // namespace Amanuensis
