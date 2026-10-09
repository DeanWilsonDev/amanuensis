#pragma once
#include "amanuensis/compat.hpp"

namespace amanuensis::json {
struct WriterOptions {
  bool pretty = true;
  int indentWidth = 2;
  char indentChar = ' ';
  bool trailingNewline = true;
};
} // namespace amanuensis::json
