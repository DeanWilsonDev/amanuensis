#include "json/parser/parselets/object-parselet.hpp"
#include "core/quoted-string/quoted-string.hpp"
#include "json/parser/value-parser.hpp"
#include "json/parser/whitespace.hpp"

#include <string>
#include <utility>

namespace amanuensis::json {

core::ParseResult ObjectParselet::Parse(core::Cursor& cursor, const ValueParser& values) const
{
  cursor.Advance();
  SkipWhitespace(cursor);

  core::Value objectValue = core::Value::MakeObject();

  if (!cursor.IsAtEnd() && cursor.Peek() == '}') {
    cursor.Advance();
    return core::ParseResult{true, std::move(objectValue), {}};
  }

  while (true) {
    SkipWhitespace(cursor);

    if (cursor.IsAtEnd() || cursor.Peek() != '"') {
      return cursor.MakeError("Expected string key in object");
    }

    auto keyResult = core::ParseQuotedString(cursor);
    if (!keyResult.succeeded) {
      return keyResult;
    }
    std::string key = keyResult.value.AsString();

    SkipWhitespace(cursor);

    if (cursor.IsAtEnd() || cursor.Peek() != ':') {
      return cursor.MakeError("Expected ':' after object key");
    }
    cursor.Advance();

    SkipWhitespace(cursor);

    auto valueResult = values.Parse(cursor);
    if (!valueResult.succeeded) {
      return valueResult;
    }

    objectValue.Insert(std::move(key), std::move(valueResult.value));

    SkipWhitespace(cursor);

    if (cursor.IsAtEnd()) {
      return cursor.MakeError("Unterminated object");
    }

    if (cursor.Peek() == '}') {
      cursor.Advance();
      return core::ParseResult{true, std::move(objectValue), {}};
    }

    if (cursor.Peek() != ',') {
      return cursor.MakeError(
          std::string("Expected ',' or '}' in object, got '") + cursor.Peek() + "'"
      );
    }
    cursor.Advance();

    // RFC 8259: no trailing commas
    SkipWhitespace(cursor);
    if (!cursor.IsAtEnd() && cursor.Peek() == '}') {
      return cursor.MakeError("Trailing comma in object");
    }
  }
}

} // namespace amanuensis::json
