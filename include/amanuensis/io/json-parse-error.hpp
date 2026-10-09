#pragma once
#include "amanuensis/compat.hpp"

#include <string>

namespace amanuensis {

struct JsonParseError {
  std::string message;
  int line;
  int column;
};
} // namespace amanuensis
