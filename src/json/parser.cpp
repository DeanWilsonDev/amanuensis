#include "parser.hpp"
#include "amanuensis/json.hpp"
#include "core/number-format.hpp"
#include "core/quoted-string.hpp"

#include <string_view>

namespace amanuensis::json {

Parser::Parser(std::string_view input)
    : cursor(input)
{
}

core::ParseResult Parser::Parse()
{
  this->SkipWhitespace();
  auto result = ParseValue();
  if (!result.succeeded)
    return result;
  this->SkipWhitespace();
  if (!cursor.IsAtEnd())
    return cursor.MakeError(
        std::string("Unexpected content after JSON value: '") + cursor.Peek() + "'"
    );
  return result;
}

// -----------------------------------------------------------------------
// Whitespace
// -----------------------------------------------------------------------

void Parser::SkipWhitespace()
{
  while (!cursor.IsAtEnd()) {
    char current = cursor.Peek();
    if (current == ' ' || current == '\t' || current == '\n' || current == '\r') {
      cursor.Advance();
    }
    else {
      break;
    }
  }
}

// -----------------------------------------------------------------------
// Literal keywords: null, true, false
// -----------------------------------------------------------------------

core::ParseResult Parser::ParseNull()
{
  const char* expected = "null";
  for (int i = 0; i < 4; ++i) {
    if (cursor.IsAtEnd() || cursor.Peek() != expected[i]) {
      return cursor.MakeError("Invalid literal, expected 'null'");
    }
    cursor.Advance();
  }
  return core::ParseResult{true, core::Value(), {}};
}

core::ParseResult Parser::ParseTrue()
{
  const char* expected = "true";
  for (int i = 0; i < 4; ++i) {
    if (cursor.IsAtEnd() || cursor.Peek() != expected[i]) {
      return cursor.MakeError("Invalid literal, expected 'true'");
    }
    cursor.Advance();
  }
  return core::ParseResult{true, core::Value(true), {}};
}

core::ParseResult Parser::ParseFalse()
{
  const char* expected = "false";
  for (int i = 0; i < 5; ++i) {
    if (cursor.IsAtEnd() || cursor.Peek() != expected[i]) {
      return cursor.MakeError("Invalid literal, expected 'false'");
    }
    cursor.Advance();
  }
  return core::ParseResult{true, core::Value(false), {}};
}

// -----------------------------------------------------------------------
// Numbers — shared with every format, see core/number-format.hpp.
// -----------------------------------------------------------------------

core::ParseResult Parser::ParseNumber()
{
  return core::ParseNumber(cursor);
}

// -----------------------------------------------------------------------
// Strings — shared with every format, see core/quoted-string.hpp.
// -----------------------------------------------------------------------

core::ParseResult Parser::ParseString()
{
  return core::ParseQuotedString(cursor);
}

core::ParseResult Parser::ParseArray()
{
  cursor.Advance(); // consume '['
  SkipWhitespace();

  core::Value arrayValue = Json::MakeArray();

  if (!cursor.IsAtEnd() && cursor.Peek() == ']') {
    cursor.Advance();
    return core::ParseResult{true, std::move(arrayValue), {}};
  }

  while (true) {
    this->SkipWhitespace();

    auto elementResult = this->ParseValue();
    if (!elementResult.succeeded) {
      return elementResult;
    }
    Json::PushBack(arrayValue, std::move(elementResult.value));

    this->SkipWhitespace();

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
    cursor.Advance(); // consume ','

    // RFC 8259: no trailing commas
    this->SkipWhitespace();
    if (!cursor.IsAtEnd() && cursor.Peek() == ']') {
      return cursor.MakeError("Trailing comma in array");
    }
  }
}

core::ParseResult Parser::ParseObject()
{
  cursor.Advance(); // consume '{'
  this->SkipWhitespace();

  core::Value objectValue = Json::MakeObject();

  if (!cursor.IsAtEnd() && cursor.Peek() == '}') {
    cursor.Advance();
    return core::ParseResult{true, std::move(objectValue), {}};
  }

  while (true) {
    this->SkipWhitespace();

    if (cursor.IsAtEnd() || cursor.Peek() != '"') {
      return cursor.MakeError("Expected string key in object");
    }

    auto keyResult = this->ParseString();
    if (!keyResult.succeeded) {
      return keyResult;
    }
    std::string key = Json::AsString(keyResult.value);

    this->SkipWhitespace();

    if (cursor.IsAtEnd() || cursor.Peek() != ':') {
      return cursor.MakeError("Expected ':' after object key");
    }
    cursor.Advance(); // consume ':'

    this->SkipWhitespace();

    auto valueResult = this->ParseValue();
    if (!valueResult.succeeded) {
      return valueResult;
    }

    Json::Insert(objectValue, std::move(key), std::move(valueResult.value));

    this->SkipWhitespace();

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
    cursor.Advance(); // consume ','

    // RFC 8259: no trailing commas
    this->SkipWhitespace();
    if (!cursor.IsAtEnd() && cursor.Peek() == '}') {
      return cursor.MakeError("Trailing comma in object");
    }
  }
}

core::ParseResult Parser::ParseValue()
{
  this->SkipWhitespace();

  if (cursor.IsAtEnd()) {
    return cursor.MakeError("Unexpected end of input");
  }

  char current = cursor.Peek();

  switch (current) {
  case 'n':
    return this->ParseNull();
  case 't':
    return this->ParseTrue();
  case 'f':
    return this->ParseFalse();
  case '"':
    return this->ParseString();
  case '[':
    return this->ParseArray();
  case '{':
    return this->ParseObject();
  default:
    if (current == '-' || (current >= '0' && current <= '9')) {
      return this->ParseNumber();
    }
    return cursor.MakeError(std::string("Unexpected character '") + current + "'");
  }
}
} // namespace amanuensis::json
