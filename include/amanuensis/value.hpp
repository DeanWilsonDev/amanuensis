#pragma once

#include <cstddef>
#include <variant>
#include "amanuensis/ordered-map.hpp"

namespace Amanuensis {

enum class ValueType { Null, Boolean, Integer, Double, String, Array, Object };

struct Value {
  using DataType = std::
      variant<std::nullptr_t, bool, long long, double, std::string, std::vector<Value>, OrderedMap>;
  DataType data;
};

} // namespace Amanuensis
