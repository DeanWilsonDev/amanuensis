#pragma once

#include <amanuensis/value.hpp>
#include <amanuensis/io/parser-error.hpp>

namespace Amanuensis {
struct ParseResult {
  bool succeeded;
  Value value;
  ParseError error;
};

} // namespace Amanuensis
