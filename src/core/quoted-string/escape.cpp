#include "core/quoted-string/escape.hpp"
#include "core/quoted-string/unicode-escape.hpp"

namespace amanuensis::core {

std::optional<ParseError> ReadEscape(Cursor& cursor, std::string& output)
{
  if (cursor.IsAtEnd()) {
    return cursor.ErrorHere("Unterminated escape sequence");
  }
  char escapeCharacter = cursor.Advance();
  switch (escapeCharacter) {
  case '"':
    output.push_back('"');
    return std::nullopt;
  case '\\':
    output.push_back('\\');
    return std::nullopt;
  case '/':
    output.push_back('/');
    return std::nullopt;
  case 'b':
    output.push_back('\b');
    return std::nullopt;
  case 'f':
    output.push_back('\f');
    return std::nullopt;
  case 'n':
    output.push_back('\n');
    return std::nullopt;
  case 'r':
    output.push_back('\r');
    return std::nullopt;
  case 't':
    output.push_back('\t');
    return std::nullopt;
  case 'u':
    return ReadUnicodeEscape(cursor, output);
  default:
    return cursor.ErrorHere(std::string("Invalid escape sequence '\\") + escapeCharacter + "'");
  }
}

} // namespace amanuensis::core
