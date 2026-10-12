#include "calamus/parser/body/body-line-parser.hpp"
#include "calamus/parser/line-cursor.hpp"

namespace amanuensis::calamus {

MaybeError BodyLineParser::Parse(const SourceLine& line, BodyLine& bodyLine) const
{
  LineCursor cursor(line);
  bodyLine.number = line.number;
  if (auto error = this->prefix.Parse(cursor, bodyLine)) {
    return error;
  }
  if (cursor.Peek() == '-') {
    return this->item.Parse(cursor, bodyLine);
  }
  return this->key.Parse(cursor, bodyLine);
}

} // namespace amanuensis::calamus
