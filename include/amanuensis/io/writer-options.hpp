#pragma once

namespace Amanuensis {
struct WriterOptions {
  bool pretty = true;
  int indentWidth = 2;
  char indentChar = ' ';
  bool trailingNewline = true;
};
} // namespace Amanuensis
