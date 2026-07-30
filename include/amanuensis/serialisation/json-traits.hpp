#pragma once
#include <amanuensis/value.hpp>
#include <amanuensis/json.hpp>

#include <optional>
#include <string>
#include <vector>
#include <map>

namespace Amanuensis {
template <typename T> struct JsonTraits; // Mechanism 3 — specialise for external types

// -----------------------------------------------------------------------
// Built-in JsonTraits for fundamental and standard-library types
// -----------------------------------------------------------------------

// bool
template <> struct JsonTraits<bool> {
  static Value ToJson(const bool& value) { return Value(value); }
  static bool FromJson(const Value& value) { return Json::AsBoolean(value); }
};

// int
template <> struct JsonTraits<int> {
  static Value ToJson(const int& value) { return Value(value); }
  static int FromJson(const Value& value) { return static_cast<int>(Json::AsInteger(value)); }
};

// long long
template <> struct JsonTraits<long long> {
  static Value ToJson(const long long& value) { return Value(value); }
  static long long FromJson(const Value& value) { return Json::AsInteger(value); }
};

// double
template <> struct JsonTraits<double> {
  static Value ToJson(const double& value) { return Value(value); }
  static double FromJson(const Value& value) { return Json::AsDouble(value); }
};

// std::string
template <> struct JsonTraits<std::string> {
  static Value ToJson(const std::string& value) { return Value(value); }
  static std::string FromJson(const Value& value) { return Json::AsString(value); }
};

// std::vector<T>
template <typename ElementType> struct JsonTraits<std::vector<ElementType>> {
  static Value ToJson(const std::vector<ElementType>& elements);
  static std::vector<ElementType> FromJson(const Value& value);
};

// std::optional<T>
template <typename WrappedType> struct JsonTraits<std::optional<WrappedType>> {
  static Value ToJson(const std::optional<WrappedType>& optionalValue);
  static std::optional<WrappedType> FromJson(const Value& value);
};

// std::map<std::string, T>
template <typename MappedType> struct JsonTraits<std::map<std::string, MappedType>> {
  static Value ToJson(const std::map<std::string, MappedType>& entries);
  static std::map<std::string, MappedType> FromJson(const Value& value);
};

// Detect JsonTraits<T>::ToJson
template <typename T, typename = void> struct HasJsonTraits : std::false_type {};

template <typename T>
struct HasJsonTraits<T, std::void_t<decltype(JsonTraits<T>::ToJson(std::declval<const T&>()))>>
    : std::true_type {};

} // namespace Amanuensis
