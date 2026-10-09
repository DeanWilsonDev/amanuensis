#pragma once
#include "amanuensis/compat.hpp"

#include <string>

namespace amanuensis {

struct ParseError {
  std::string message;
  int line;
  int column;
};
} // namespace amanuensis
