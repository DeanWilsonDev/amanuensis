#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include "amanuensis/json-value.hpp"
#include "amanuensis/object-iterator.hpp"

namespace Amanuensis {

class Json {
public:
  static JsonValueType GetType(const JsonValue& value);
  static bool IsNull(const JsonValue& value);
  static bool IsBoolean(const JsonValue& value);
  static bool IsInteger(const JsonValue& value);
  static bool IsDouble(const JsonValue& value);
  static bool IsNumber(const JsonValue& value);
  static bool IsString(const JsonValue& value);
  static bool IsArray(const JsonValue& value);
  static bool IsObject(const JsonValue& value);

  static bool AsBoolean(const JsonValue& value);
  static long long AsInteger(const JsonValue& value);
  static double AsDouble(const JsonValue& value);
  static const std::string& AsString(const JsonValue& value);
  static const std::vector<JsonValue>& AsArray(const JsonValue& value);

  static void PushBack(JsonValue& value, JsonValue element);
  static std::size_t Size(const JsonValue& value);
  static const JsonValue& At(const JsonValue& value, std::size_t index);
  static JsonValue& At(JsonValue& value, std::size_t index);

  static void Insert(JsonValue& value, std::string key, JsonValue element);
  static bool Contains(const JsonValue& value, const std::string& key);
  static const JsonValue& Get(const JsonValue& value, const std::string& key);
  static JsonValue& Get(JsonValue& value, const std::string& key);
  static const JsonValue* Find(const JsonValue& value, const std::string& key);

  static ObjectIterator BeginObject(const JsonValue& value);
  static ObjectIterator EndObject(const JsonValue& value);

  static JsonValue MakeArray();
  static JsonValue MakeObject();
};

} // namespace Amanuensis
