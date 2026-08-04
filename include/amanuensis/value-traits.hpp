#pragma once

#include <vector>
#include "ordered-map.hpp"

namespace Amanuensis {

template <
    typename TValue,
    typename TValueArray = std::vector<TValue>,
    typename TValueObject = Amanuensis::OrderedMap<TValue>>
struct ValueTraits {
  using Value = TValue;
  using ValueArray = TValueArray;
  using ValueObject = TValueObject;

  static TValue MakeNull() { return TValue{std::monostate{}}; }
  static TValue MakeBoolean(bool value) { return TValue{value}; }
  static TValue MakeInteger(long long value) { return TValue{value}; }
  static TValue MakeDouble(double value) { return TValue{value}; }
  static TValue MakeString(const std::string& value) { return TValue{value}; }
  static TValue MakeArray() { return TValue{TValueArray{}}; }
  static TValue MakeObject() { return TValue{TValueObject{}}; }

  static void PushBack(TValue& target, TValue element)
  {
    std::get<TValueArray>(target.data).push_back(std::move(element));
  }

  static void Insert(TValue& target, const std::string& key, TValue element)
  {
    std::get<TValueObject>(target.data).Insert(key, std::move(element));
  }
};
} // namespace Amanuensis
