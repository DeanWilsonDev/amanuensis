#include "core/quoted-string/escape.hpp"
#include "core/quoted-string/quoted-string.hpp"

#include <utility>

namespace amanuensis::core {

ParseResult ParseQuotedString(Cursor& cursor)
{
  if (cursor.IsAtEnd() || cursor.Peek() != '"') {
    return cursor.MakeError("Expected '\"' at start of string");
  }
  cursor.Advance();

  std::string result;
  while (true) {
    if (cursor.IsAtEnd()) {
      return cursor.MakeError("Unterminated string literal");
    }

    char current = cursor.Peek();
    if (current == '"') {
      cursor.Advance();
      return ParseResult{true, Value(std::move(result)), {}};
    }
    if (static_cast<unsigned char>(current) < 0x20) {
      return cursor.MakeError("Unescaped control character in string");
    }
    if (current != '\\') {
      result.push_back(cursor.Advance());
      continue;
    }

    cursor.Advance();
    if (auto error = ReadEscape(cursor, result)) {
      return ParseResult{false, Value(), std::move(*error)};
    }
  }
}

} // namespace amanuensis::core
