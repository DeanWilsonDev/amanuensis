#pragma once

#include <string>

namespace Amanuensis {

struct ParseError {
  std::string message;
  int line;
  int column;
};
} // namespace Amanuensis
