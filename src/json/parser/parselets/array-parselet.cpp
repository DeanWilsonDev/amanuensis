#include "json/parser/parselets/array-parselet.hpp"
#include "json/parser/value-parser.hpp"
#include "json/parser/whitespace.hpp"

#include <string>
#include <utility>

namespace amanuensis::json {

core::ParseResult ArrayParselet::Parse(core::Cursor& cursor, const ValueParser& values) const
{
  cursor.Advance();
  SkipWhitespace(cursor);

  core::Value arrayValue = core::Value::MakeArray();

  if (!cursor.IsAtEnd() && cursor.Peek() == ']') {
    cursor.Advance();
    return core::ParseResult{true, std::move(arrayValue), {}};
  }

  while (true) {
    SkipWhitespace(cursor);

    auto elementResult = values.Parse(cursor);
    if (!elementResult.succeeded) {
      return elementResult;
    }
    arrayValue.PushBack(std::move(elementResult.value));

    SkipWhitespace(cursor);

    if (cursor.IsAtEnd()) {
      return cursor.MakeError("Unterminated array");
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

    // RFC 8259: no trailing commas
    SkipWhitespace(cursor);
    if (!cursor.IsAtEnd() && cursor.Peek() == ']') {
      return cursor.MakeError("Trailing comma in array");
    }
  }
}

} // namespace amanuensis::json
