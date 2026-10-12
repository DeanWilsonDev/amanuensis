#include "json/parser/whitespace.hpp"

namespace amanuensis::json {

void SkipWhitespace(core::Cursor& cursor)
{
  while (!cursor.IsAtEnd()) {
    char current = cursor.Peek();
    if (current == ' ' || current == '\t' || current == '\n' || current == '\r') {
      cursor.Advance();
    }
    else {
      break;
    }
  }
}

} // namespace amanuensis::json
