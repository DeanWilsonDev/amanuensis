#include "calamus/parser/body/line-prefix-parser.hpp"

namespace amanuensis::calamus {

MaybeError LinePrefixParser::Parse(LineCursor& cursor, BodyLine& bodyLine) const
{
  if (auto error = this->ParseMarker(cursor, bodyLine)) {
    return error;
  }
  return this->ParseIndentation(cursor, bodyLine);
}

MaybeError LinePrefixParser::ParseMarker(LineCursor& cursor, BodyLine& bodyLine) const
{
  char marker = cursor.Peek();
  if (marker == '@') {
    return cursor.ErrorAt("Header lines must come before the body", 1);
  }
  if (marker == '\t') {
    return cursor.ErrorAt("Tabs aren't allowed in indentation", 1);
  }
  if (marker != '*' && marker != ' ') {
    return cursor.ErrorAt("Expected '*' or a space in the marker column", 1);
  }
  bodyLine.marked = marker == '*';
  cursor.Advance();

  if (cursor.IsAtEnd()) {
    return cursor.ErrorAt("Expected a field after the marker", 2);
  }
  if (cursor.Peek() == '\t') {
    return cursor.ErrorAt("Tabs aren't allowed in indentation", 2);
  }
  if (cursor.Peek() != ' ') {
    return cursor.ErrorAt("Expected a space after the marker column", 2);
  }
  cursor.Advance();
  return std::nullopt;
}

MaybeError LinePrefixParser::ParseIndentation(LineCursor& cursor, BodyLine& bodyLine) const
{
  int spaces = 0;
  while (cursor.Peek() == ' ' || cursor.Peek() == '\t') {
    if (cursor.Peek() == '\t') {
      return cursor.ErrorHere("Tabs aren't allowed in indentation");
    }
    ++spaces;
    cursor.Advance();
  }
  if (cursor.IsAtEnd()) {
    return cursor.ErrorHere("Expected a field after the marker");
  }
  if (spaces % 2 != 0) {
    return cursor.ErrorHere("Indentation must be a multiple of two spaces");
  }
  bodyLine.depth = spaces / 2;
  return std::nullopt;
}

} // namespace amanuensis::calamus
