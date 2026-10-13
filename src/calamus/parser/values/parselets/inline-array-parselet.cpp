#include "calamus/parser/values/parselets/inline-array-parselet.hpp"
#include "calamus/parser/values/value-parser.hpp"

#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace amanuensis::calamus {

core::ParseResult InlineArrayParselet::Parse(LineCursor& cursor, const ValueParser& values) const
{
  cursor.Advance();
  cursor.SkipSpaces();

  core::Value arrayValue{std::vector<core::Value>{}};
  auto& elements = std::get<std::vector<core::Value>>(arrayValue.data);

  if (cursor.Peek() == ']') {
    cursor.Advance();
    return core::ParseResult{true, std::move(arrayValue), {}};
  }

  while (true) {
    auto elementResult = values.Parse(cursor);
    if (!elementResult.succeeded) {
      return elementResult;
    }
    elements.push_back(std::move(elementResult.value));

    cursor.SkipSpaces();

    if (cursor.IsAtEnd()) {
      return cursor.MakeError("Unterminated array; an inline array must end on its own line");
    }
    if (cursor.Peek() == ']') {
      cursor.Advance();
      return core::ParseResult{true, std::move(arrayValue), {}};
    }
    if (cursor.Peek() != ',') {
      return cursor.MakeError(
          std::string("Expected ',' or ']' in array, got '") + cursor.Peek() + "'"
      );
    }
    cursor.Advance();

    cursor.SkipSpaces();
    if (cursor.Peek() == ']') {
      return cursor.MakeError("Trailing comma in array");
    }
  }
}

} // namespace amanuensis::calamus
