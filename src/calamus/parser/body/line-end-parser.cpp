#include "calamus/parser/body/line-end-parser.hpp"
#include "calamus/parser/messages.hpp"

#include <string>
#include <utility>

namespace amanuensis::calamus {

MaybeError LineEndParser::Parse(LineCursor& cursor, BodyLine& bodyLine, const char* after) const
{
  cursor.SkipSpaces();
  if (cursor.IsAtEnd()) {
    return std::nullopt;
  }
  if (cursor.Peek() == '|' && bodyLine.kind == LineKind::Field && !bodyLine.recordedDefault) {
    cursor.Advance();
    cursor.SkipSpaces();
    if (cursor.IsAtEnd()) {
      return cursor.ErrorHere("Expected a recorded default after '|'");
    }
    auto defaultResult = this->values.Parse(cursor);
    if (!defaultResult.succeeded) {
      return defaultResult.error;
    }
    bodyLine.recordedDefault = std::move(defaultResult.value);
    return this->Parse(cursor, bodyLine, "the recorded default");
  }
  if (cursor.Peek() == '|' && bodyLine.kind == LineKind::Item) {
    return cursor.ErrorHere("A recorded default can only follow a field's value");
  }
  if (cursor.Peek() == '#') {
    return cursor.ErrorHere(CommentError);
  }
  return cursor.ErrorHere(std::string("Unexpected '") + cursor.Peek() + "' after " + after);
}

} // namespace amanuensis::calamus
