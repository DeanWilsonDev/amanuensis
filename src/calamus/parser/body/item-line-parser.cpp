#include "calamus/parser/body/item-line-parser.hpp"

#include <utility>

namespace amanuensis::calamus {

MaybeError ItemLineParser::Parse(LineCursor& cursor, BodyLine& bodyLine) const
{
  cursor.Advance();
  if (cursor.IsAtEnd()) {
    bodyLine.kind = LineKind::ItemBlock;
    return std::nullopt;
  }
  if (cursor.Peek() != ' ') {
    return cursor.ErrorHere("Expected a space after '-'");
  }
  cursor.SkipSpaces();
  bodyLine.kind = LineKind::Item;
  auto itemResult = this->values.Parse(cursor);
  if (!itemResult.succeeded) {
    return itemResult.error;
  }
  bodyLine.value = std::move(itemResult.value);
  return this->lineEnd.Parse(cursor, bodyLine, "the value");
}

} // namespace amanuensis::calamus
