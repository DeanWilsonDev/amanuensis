#pragma once

#include <cstddef>
#include <vector>
#include <variant>
#include "amanuensis/ordered-map.hpp"

namespace Amanuensis {

enum class JsonValueType { Null, Boolean, Integer, Double, String, Array, Object };

struct JsonValue {
  using DataType = std::variant<
      std::monostate,
      bool,
      long long,
      double,
      std::string,
      std::vector<JsonValue>,
      OrderedMap<JsonValue>>;
  DataType data;
};

} // namespace Amanuensis
