#pragma once

#include <string>

namespace Amanuensis {

struct JsonParseError {
  std::string message;
  int line;
  int column;
};
} // namespace Amanuensis
