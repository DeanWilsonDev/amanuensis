#pragma once

#include <string>
#include <utility>
#include <variant>
#include <vector>
#include "amanuensis/errors.hpp"
#include "amanuensis/json-value.hpp"
#include "amanuensis/ordered-map.hpp"

namespace Amanuensis {

template <
    typename TValue,
    typename TValueArray = std::vector<TValue>,
    typename TValueObject = Amanuensis::OrderedMap<TValue>>
struct ValueTraits {
  using Value = TValue;
  using ValueArray = TValueArray;
  using ValueObject = TValueObject;

  static JsonValueType GetType(const TValue& value)
  {
    switch (value.data.index()) {
    case 0:
      return JsonValueType::Null;
    case 1:
      return JsonValueType::Boolean;
    case 2:
      return JsonValueType::Integer;
    case 3:
      return JsonValueType::Double;
    case 4:
      return JsonValueType::String;
    case 5:
      return JsonValueType::Array;
    case 6:
      return JsonValueType::Object;
    default:
      throw TypeMismatchError("Unsupported value type");
    }
  }

  static bool AsBoolean(const TValue& value) { return std::get<bool>(value.data); }
  static long long AsInteger(const TValue& value) { return std::get<long long>(value.data); }
  static double AsDouble(const TValue& value) { return std::get<double>(value.data); }
  static const std::string& AsString(const TValue& value)
  {
    return std::get<std::string>(value.data);
  }
  static const TValueArray& AsArray(const TValue& value)
  {
    return std::get<TValueArray>(value.data);
  }
  static const std::vector<std::pair<std::string, TValue>>& AsObject(const TValue& value)
  {
    return std::get<TValueObject>(value.data).GetEntries();
  }

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
