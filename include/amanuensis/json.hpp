#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include "amanuensis/value.hpp"
#include "amanuensis/object-iterator.hpp"

namespace Amanuensis {

class Json {
public:
  static ValueType GetType(const Value& value);
  static bool IsNull(const Value& value);
  static bool IsBoolean(const Value& value);
  static bool IsInteger(const Value& value);
  static bool IsDouble(const Value& value);
  static bool IsNumber(const Value& value);
  static bool IsString(const Value& value);
  static bool IsArray(const Value& value);
  static bool IsObject(const Value& value);

  static bool AsBoolean(const Value& value);
  static long long AsInteger(const Value& value);
  static double AsDouble(const Value& value);
  static const std::string& AsString(const Value& value);
  static const std::vector<Value>& AsArray(const Value& value);

  static void PushBack(Value& value, Value element);
  static std::size_t Size(const Value& value);
  static const Value& At(const Value& value, std::size_t index);
  static Value& At(Value& value, std::size_t index);

  static void Insert(Value& value, std::string key, Value element);
  static bool Contains(const Value& value, const std::string& key);
  static const Value& Get(const Value& value, const std::string& key);
  static Value& Get(Value& value, const std::string& key);
  static const Value* Find(const Value& value, const std::string& key);

  static ObjectIterator BeginObject(const Value& value);
  static ObjectIterator EndObject(const Value& value);

  static Value MakeArray();
  static Value MakeObject();
};

} // namespace Amanuensis
