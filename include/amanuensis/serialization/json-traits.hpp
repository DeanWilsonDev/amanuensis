#pragma once
#include <amanuensis/json-value.hpp>
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
  static JsonValue ToJson(const bool& value) { return JsonValue(value); }
  static bool FromJson(const JsonValue& value) { return Json::AsBoolean(value); }
};

// int
template <> struct JsonTraits<int> {
  static JsonValue ToJson(const int& value) { return JsonValue(value); }
  static int FromJson(const JsonValue& value) { return static_cast<int>(Json::AsInteger(value)); }
};

// long long
template <> struct JsonTraits<long long> {
  static JsonValue ToJson(const long long& value) { return JsonValue(value); }
  static long long FromJson(const JsonValue& value) { return Json::AsInteger(value); }
};

// double
template <> struct JsonTraits<double> {
  static JsonValue ToJson(const double& value) { return JsonValue(value); }
  static double FromJson(const JsonValue& value) { return Json::AsDouble(value); }
};

// std::string
template <> struct JsonTraits<std::string> {
  static JsonValue ToJson(const std::string& value) { return JsonValue(value); }
  static std::string FromJson(const JsonValue& value) { return Json::AsString(value); }
};

// std::vector<T>
template <typename ElementType> struct JsonTraits<std::vector<ElementType>> {
  static JsonValue ToJson(const std::vector<ElementType>& elements);
  static std::vector<ElementType> FromJson(const JsonValue& value);
};

// std::optional<T>
template <typename WrappedType> struct JsonTraits<std::optional<WrappedType>> {
  static JsonValue ToJson(const std::optional<WrappedType>& optionalJsonValue);
  static std::optional<WrappedType> FromJson(const JsonValue& value);
};

// std::map<std::string, T>
template <typename MappedType> struct JsonTraits<std::map<std::string, MappedType>> {
  static JsonValue ToJson(const std::map<std::string, MappedType>& entries);
  static std::map<std::string, MappedType> FromJson(const JsonValue& value);
};

// Detect JsonTraits<T>::ToJson
template <typename T, typename = void> struct HasJsonTraits : std::false_type {};

template <typename T>
struct HasJsonTraits<T, std::void_t<decltype(JsonTraits<T>::ToJson(std::declval<const T&>()))>>
    : std::true_type {};

} // namespace Amanuensis
