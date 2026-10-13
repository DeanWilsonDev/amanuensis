#pragma once

#include <amanuensis/core/value.hpp>
#include <amanuensis/core/parse-error.hpp>

namespace amanuensis::core {
struct ParseResult {
  bool succeeded;
  Value value;
  ParseError error;
};

} // namespace amanuensis::core
