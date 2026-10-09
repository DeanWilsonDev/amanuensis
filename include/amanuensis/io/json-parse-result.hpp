#pragma once

#include <amanuensis/json-value.hpp>
#include <amanuensis/io/json-parse-error.hpp>

namespace amanuensis {
struct JsonParseResult {
  bool succeeded;
  JsonValue value;
  JsonParseError error;
};

} // namespace amanuensis
