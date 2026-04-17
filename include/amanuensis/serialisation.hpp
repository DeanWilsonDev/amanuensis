#pragma once

#include "value.hpp"

namespace Amanuensis {

template <typename T> Value ToJson(const T& value);
template <typename T> T FromJson(const Value& value);

template <typename T> struct FromJsonResult {
  bool succeeded;
  T value;
  std::string errorMessage;
};

template <typename T> FromJsonResult<T> TryFromJson(const Value& value);

} // namespace Amanuensis
