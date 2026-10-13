#include "calamus/parser/values/value-parser.hpp"

#include <string>

namespace amanuensis::calamus {

core::ParseResult ValueParser::Parse(LineCursor& cursor) const
{
  if (cursor.IsAtEnd()) {
    return cursor.MakeError("Expected a value");
  }
  const ValueParselet* parselet = this->parselets.Find(cursor.Peek());
  if (parselet == nullptr) {
    return cursor.MakeError(std::string("Unexpected character '") + cursor.Peek() + "'");
  }
  return parselet->Parse(cursor, *this);
}

} // namespace amanuensis::calamus
