#include "calamus/parser/header/parselets/base-field-parselet.hpp"
#include "core/quoted-string.hpp"

#include <string>
#include <variant>

namespace amanuensis::calamus {

MaybeError BaseFieldParselet::Parse(LineCursor& cursor, Header& header) const
{
  if (cursor.Peek() != '"') {
    return cursor.ErrorHere("@base must be a quoted string");
  }
  auto baseResult = core::ParseQuotedString(cursor);
  if (!baseResult.succeeded) {
    return baseResult.error;
  }
  cursor.SkipSpaces();
  if (!cursor.IsAtEnd()) {
    return cursor.ErrorHere("Unexpected text after the @base string");
  }
  header.base = std::get<std::string>(baseResult.value.data);
  return std::nullopt;
}

} // namespace amanuensis::calamus
