#pragma once
#include <amanuensis/value.hpp>
#include <amanuensis/json.hpp>

#include <optional>
#include <string>
#include <vector>
#include <map>

namespace amanuensis {
#ifdef AMANUENSIS_NO_COMPAT
template <typename T> struct SerialTraits; // Mechanism 3 — specialise for external types
#else
// Mechanism 3 — specialise for external types. Until the consumer sweep (see
// compat.hpp), the primary template forwards to an old JsonTraits<T>
// specialisation, so one written against the old names still works. FromValue
// deduces its return type so that a T no function can return, such as an
// array, still gets the "no serialisation" static_assert and nothing else.
template <typename T> struct SerialTraits {
  static Value ToValue(const T& value)
    requires requires(const T& source) { JsonTraits<T>::ToJson(source); }
  {
    return JsonTraits<T>::ToJson(value);
  }

  static auto FromValue(const Value& value)
    requires requires(const Value& source) { JsonTraits<T>::FromJson(source); }
  {
    return JsonTraits<T>::FromJson(value);
  }
};
#endif

// -----------------------------------------------------------------------
// Built-in SerialTraits for fundamental and standard-library types
// -----------------------------------------------------------------------

// bool
template <> struct SerialTraits<bool> {
  static Value ToValue(const bool& value) { return Value(value); }
  static bool FromValue(const Value& value) { return Json::AsBoolean(value); }
};

// int
template <> struct SerialTraits<int> {
  static Value ToValue(const int& value) { return Value(value); }
  static int FromValue(const Value& value) { return static_cast<int>(Json::AsInteger(value)); }
};

// long long
template <> struct SerialTraits<long long> {
  static Value ToValue(const long long& value) { return Value(value); }
  static long long FromValue(const Value& value) { return Json::AsInteger(value); }
};

// double
template <> struct SerialTraits<double> {
  static Value ToValue(const double& value) { return Value(value); }
  static double FromValue(const Value& value) { return Json::AsDouble(value); }
};

// std::string
template <> struct SerialTraits<std::string> {
  static Value ToValue(const std::string& value) { return Value(value); }
  static std::string FromValue(const Value& value) { return Json::AsString(value); }
};

// std::vector<T>
template <typename ElementType> struct SerialTraits<std::vector<ElementType>> {
  static Value ToValue(const std::vector<ElementType>& elements);
  static std::vector<ElementType> FromValue(const Value& value);
};

// std::optional<T>
template <typename WrappedType> struct SerialTraits<std::optional<WrappedType>> {
  static Value ToValue(const std::optional<WrappedType>& optionalValue);
  static std::optional<WrappedType> FromValue(const Value& value);
};

// std::map<std::string, T>
template <typename MappedType> struct SerialTraits<std::map<std::string, MappedType>> {
  static Value ToValue(const std::map<std::string, MappedType>& entries);
  static std::map<std::string, MappedType> FromValue(const Value& value);
};

// Detect SerialTraits<T>::ToValue
template <typename T, typename = void> struct HasSerialTraits : std::false_type {};

template <typename T>
struct HasSerialTraits<T, std::void_t<decltype(SerialTraits<T>::ToValue(std::declval<const T&>()))>>
    : std::true_type {};

#ifndef AMANUENSIS_NO_COMPAT
// Old name, kept until the consumer sweep (see compat.hpp).
template <typename T> using HasJsonTraits = HasSerialTraits<T>;
#endif

} // namespace amanuensis
