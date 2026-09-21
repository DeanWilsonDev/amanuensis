#pragma once

#include <amanuensis/json-value.hpp>
#include <amanuensis/io/json-parse-error.hpp>

namespace Amanuensis {
struct JsonParseResult {
  bool succeeded;
  JsonValue value;
  JsonParseError error;
};

} // namespace Amanuensis
