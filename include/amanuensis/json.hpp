#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>
#include "amanuensis/core/value.hpp"

#ifndef AMANUENSIS_NO_COMPAT
namespace amanuensis {

// Old name, kept until the consumer sweep (see compat.hpp). The operations on
// a Value are now its member functions: Json::Get(value, key) is
// value.Get(key), and Json::MakeObject() is core::Value::MakeObject().
class Json {
public:
  static core::ValueType GetType(const core::Value& value) { return value.GetType(); }
  static bool IsNull(const core::Value& value) { return value.IsNull(); }
  static bool IsBoolean(const core::Value& value) { return value.IsBoolean(); }
  static bool IsInteger(const core::Value& value) { return value.IsInteger(); }
  static bool IsDouble(const core::Value& value) { return value.IsDouble(); }
  static bool IsNumber(const core::Value& value) { return value.IsNumber(); }
  static bool IsString(const core::Value& value) { return value.IsString(); }
  static bool IsArray(const core::Value& value) { return value.IsArray(); }
  static bool IsObject(const core::Value& value) { return value.IsObject(); }

  static bool AsBoolean(const core::Value& value) { return value.AsBoolean(); }
  static long long AsInteger(const core::Value& value) { return value.AsInteger(); }
  static double AsDouble(const core::Value& value) { return value.AsDouble(); }
  static const std::string& AsString(const core::Value& value) { return value.AsString(); }
  static const std::vector<core::Value>& AsArray(const core::Value& value)
  {
    return value.AsArray();
  }

  static void PushBack(core::Value& value, core::Value element)
  {
    value.PushBack(std::move(element));
  }
  static std::size_t Size(const core::Value& value) { return value.Size(); }
  static const core::Value& At(const core::Value& value, std::size_t index)
  {
    return value.At(index);
  }
  static core::Value& At(core::Value& value, std::size_t index) { return value.At(index); }

  static void Insert(core::Value& value, std::string key, core::Value element)
  {
    value.Insert(std::move(key), std::move(element));
  }
  static bool Contains(const core::Value& value, const std::string& key)
  {
    return value.Contains(key);
  }
  static const core::Value& Get(const core::Value& value, const std::string& key)
  {
    return value.Get(key);
  }
  static core::Value& Get(core::Value& value, const std::string& key) { return value.Get(key); }
  static const core::Value* Find(const core::Value& value, const std::string& key)
  {
    return value.Find(key);
  }

  static core::ObjectIterator BeginObject(const core::Value& value) { return value.BeginObject(); }
  static core::ObjectIterator EndObject(const core::Value& value) { return value.EndObject(); }

  static core::Value MakeArray() { return core::Value::MakeArray(); }
  static core::Value MakeObject() { return core::Value::MakeObject(); }
};

} // namespace amanuensis
#endif
