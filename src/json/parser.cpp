#include "parser.hpp"
#include "amanuensis/json.hpp"
#include "core/quoted-string.hpp"

#include <charconv>
#include <string_view>
#include <cctype>

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
// Numbers
//
// Strategy: identify the span of characters that form the number literal,
// then decide whether it's integer or double based on the presence of
// '.', 'e', or 'E'.  Parse accordingly.
// -----------------------------------------------------------------------

core::ParseResult Parser::ParseNumber()
{
  std::size_t startPosition = cursor.Position();
  int startLine = cursor.Line();
  int startColumn = cursor.Column();

  bool isFloatingPoint = false;

  // Optional leading minus
  if (cursor.Peek() == '-') {
    cursor.Advance();
  }

  // Integer part
  if (cursor.IsAtEnd() || !std::isdigit(static_cast<unsigned char>(cursor.Peek()))) {
    return cursor.MakeError("Expected digit after '-'");
  }

  if (cursor.Peek() == '0') {
    cursor.Advance();
    // Leading zero must not be followed by another digit (RFC 8259)
    if (!cursor.IsAtEnd() && std::isdigit(static_cast<unsigned char>(cursor.Peek()))) {
      return cursor.MakeError("Leading zeros are not allowed in numbers");
    }
  }
  else {
    while (!cursor.IsAtEnd() && std::isdigit(static_cast<unsigned char>(cursor.Peek()))) {
      cursor.Advance();
    }
  }

  // Fractional part
  if (!cursor.IsAtEnd() && cursor.Peek() == '.') {
    isFloatingPoint = true;
    cursor.Advance();
    if (cursor.IsAtEnd() || !std::isdigit(static_cast<unsigned char>(cursor.Peek()))) {
      return cursor.MakeError("Expected digit after decimal point");
    }
    while (!cursor.IsAtEnd() && std::isdigit(static_cast<unsigned char>(cursor.Peek()))) {
      cursor.Advance();
    }
  }

  // Exponent part
  if (!cursor.IsAtEnd() && (cursor.Peek() == 'e' || cursor.Peek() == 'E')) {
    isFloatingPoint = true;
    cursor.Advance();
    if (!cursor.IsAtEnd() && (cursor.Peek() == '+' || cursor.Peek() == '-')) {
      cursor.Advance();
    }
    if (cursor.IsAtEnd() || !std::isdigit(static_cast<unsigned char>(cursor.Peek()))) {
      return cursor.MakeError("Expected digit in exponent");
    }
    while (!cursor.IsAtEnd() && std::isdigit(static_cast<unsigned char>(cursor.Peek()))) {
      cursor.Advance();
    }
  }

  std::string_view numberText = cursor.Slice(startPosition);

  if (isFloatingPoint) {
    // Parse as double
    double doubleValue = 0.0;
    auto [endPointer, errorCode] =
        std::from_chars(numberText.data(), numberText.data() + numberText.size(), doubleValue);
    if (errorCode != std::errc()) {
      return core::ParseResult{
          false, core::Value(),
          core::ParseError{"Failed to parse floating-point number", startLine, startColumn}
      };
    }
    return core::ParseResult{true, core::Value(doubleValue), {}};
  }
  else {
    // Try integer first; fall back to double on overflow
    long long integerValue = 0;
    auto [endPointer, errorCode] =
        std::from_chars(numberText.data(), numberText.data() + numberText.size(), integerValue);
    if (errorCode == std::errc::result_out_of_range) {
      // Overflow — fall back to double silently, as per design doc
      double fallbackDouble = 0.0;
      auto [dblEnd, dblErr] =
          std::from_chars(numberText.data(), numberText.data() + numberText.size(), fallbackDouble);
      if (dblErr != std::errc()) {
        return core::ParseResult{
            false, core::Value(),
            core::ParseError{"Failed to parse number (overflow)", startLine, startColumn}
        };
      }
      return core::ParseResult{true, core::Value(fallbackDouble), {}};
    }
    if (errorCode != std::errc()) {
      return core::ParseResult{
          false, core::Value(), core::ParseError{"Failed to parse integer", startLine, startColumn}
      };
    }
    return core::ParseResult{true, core::Value(integerValue), {}};
  }
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
