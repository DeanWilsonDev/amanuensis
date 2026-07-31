#pragma once

#include <amanuensis/value.hpp>
#include <amanuensis/io/parse-error.hpp>

namespace Amanuensis {
struct ParseResult {
  bool succeeded;
  Value value;
  ParseError error;
};

} // namespace Amanuensis
