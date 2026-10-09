#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include "amanuensis/core/value.hpp"
#include "amanuensis/core/object-iterator.hpp"

namespace amanuensis {

class Json {
public:
  static core::ValueType GetType(const core::Value& value);
  static bool IsNull(const core::Value& value);
  static bool IsBoolean(const core::Value& value);
  static bool IsInteger(const core::Value& value);
  static bool IsDouble(const core::Value& value);
  static bool IsNumber(const core::Value& value);
  static bool IsString(const core::Value& value);
  static bool IsArray(const core::Value& value);
  static bool IsObject(const core::Value& value);

  static bool AsBoolean(const core::Value& value);
  static long long AsInteger(const core::Value& value);
  static double AsDouble(const core::Value& value);
  static const std::string& AsString(const core::Value& value);
  static const std::vector<core::Value>& AsArray(const core::Value& value);

  static void PushBack(core::Value& value, core::Value element);
  static std::size_t Size(const core::Value& value);
  static const core::Value& At(const core::Value& value, std::size_t index);
  static core::Value& At(core::Value& value, std::size_t index);

  static void Insert(core::Value& value, std::string key, core::Value element);
  static bool Contains(const core::Value& value, const std::string& key);
  static const core::Value& Get(const core::Value& value, const std::string& key);
  static core::Value& Get(core::Value& value, const std::string& key);
  static const core::Value* Find(const core::Value& value, const std::string& key);

  static core::ObjectIterator BeginObject(const core::Value& value);
  static core::ObjectIterator EndObject(const core::Value& value);

  static core::Value MakeArray();
  static core::Value MakeObject();
};

} // namespace amanuensis
