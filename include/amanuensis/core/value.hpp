#pragma once

#include <cstddef>
#include <vector>
#include <variant>
#include "amanuensis/core/ordered-map.hpp"

namespace amanuensis::core {

enum class ValueType { Null, Boolean, Integer, Double, String, Array, Object };

struct Value {
  using DataType = std::variant<
      std::monostate,
      bool,
      long long,
      double,
      std::string,
      std::vector<Value>,
      OrderedMap<Value>>;
  DataType data;
};

} // namespace amanuensis::core
