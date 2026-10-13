#include "json/parser/value-parser.hpp"
#include "json/parser/whitespace.hpp"

#include <string>

namespace amanuensis::json {

core::ParseResult ValueParser::Parse(core::Cursor& cursor) const
{
  SkipWhitespace(cursor);

  if (cursor.IsAtEnd()) {
    return cursor.MakeError("Unexpected end of input");
  }

  const ValueParselet* parselet = this->parselets.Find(cursor.Peek());
  if (parselet == nullptr) {
    return cursor.MakeError(std::string("Unexpected character '") + cursor.Peek() + "'");
  }
  return parselet->Parse(cursor, *this);
}

} // namespace amanuensis::json
