#include "calamus/parser/body/key-line-parser.hpp"
#include "calamus/parser/messages.hpp"
#include "calamus/syntax.hpp"
#include "core/quoted-string.hpp"

#include <string>
#include <utility>
#include <variant>

namespace amanuensis::calamus {

MaybeError KeyLineParser::Parse(LineCursor& cursor, BodyLine& bodyLine) const
{
  bodyLine.keyColumn = cursor.Column();
  if (cursor.Peek() == '"') {
    auto keyResult = core::ParseQuotedString(cursor);
    if (!keyResult.succeeded) {
      return keyResult.error;
    }
    bodyLine.key = std::get<std::string>(keyResult.value.data);
  }
  else if (IsIdentifierStart(cursor.Peek())) {
    bodyLine.key = std::string(cursor.ReadIdentifier());
  }
  else if (cursor.Peek() == '#') {
    return cursor.ErrorHere(CommentError);
  }
  else {
    return cursor.ErrorHere("Expected a key or a '-' item");
  }

  cursor.SkipSpaces();
  if (cursor.Peek() != ':') {
    return cursor.ErrorHere("Expected ':' after the key");
  }
  cursor.Advance();
  cursor.SkipSpaces();

  if (cursor.IsAtEnd()) {
    bodyLine.kind = LineKind::Block;
    return std::nullopt;
  }

  bodyLine.kind = LineKind::Field;
  auto valueResult = this->values.Parse(cursor);
  if (!valueResult.succeeded) {
    return valueResult.error;
  }
  bodyLine.value = std::move(valueResult.value);
  return this->lineEnd.Parse(cursor, bodyLine, "the value");
}

} // namespace amanuensis::calamus
