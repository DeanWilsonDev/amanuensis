#include "amanuensis/parser.hpp"

#include <charconv>
#include <cstdlib>

namespace Amanuensis {

Parser::Parser() {}

void Parser::SetInput(std::string_view input){
  this->input = input;
}

bool Parser::IsAtEnd() const
{
  return position >= input.size();
}

char Parser::Peek() const
{
  if (IsAtEnd()) {
    return '\0';
  }
  return input[position];
}

char Parser::Advance()
{
  char current = input[position];
  ++position;
  if (current == '\n') {
    ++line;
    column = 1;
  }
  else {
    ++column;
  }
  return current;
}

ParseResult Parser::MakeError(const std::string& message) const
{
  return ParseResult{false, Value(), ParseError{message, line, column}};
}

// -----------------------------------------------------------------------
// Whitespace
// -----------------------------------------------------------------------

void Parser::SkipWhitespace()
{
  while (!this->IsAtEnd()) {
    char current = this->Peek();
    if (current == ' ' || current == '\t' || current == '\n' || current == '\r') {
      this->Advance();
    }
    else {
      break;
    }
  }
}

// -----------------------------------------------------------------------
// Literal keywords: null, true, false
// -----------------------------------------------------------------------

ParseResult Parser::ParseNull()
{
  const char* expected = "null";
  for (int i = 0; i < 4; ++i) {
    if (this->IsAtEnd() || this->Peek() != expected[i]) {
      return this->MakeError("Invalid literal, expected 'null'");
    }
    this->Advance();
  }
  return ParseResult{true, Value(), {}};
}

ParseResult Parser::ParseTrue()
{
  const char* expected = "true";
  for (int i = 0; i < 4; ++i) {
    if (this->IsAtEnd() || this->Peek() != expected[i]) {
      return this->MakeError("Invalid literal, expected 'true'");
    }
    this->Advance();
  }
  return ParseResult{true, Value(true), {}};
}

ParseResult Parser::ParseFalse()
{
  const char* expected = "false";
  for (int i = 0; i < 5; ++i) {
    if (this->IsAtEnd() || this->Peek() != expected[i]) {
      return this->MakeError("Invalid literal, expected 'false'");
    }
    this->Advance();
  }
  return ParseResult{true, Value(false), {}};
}

// -----------------------------------------------------------------------
// Numbers
//
// Strategy: identify the span of characters that form the number literal,
// then decide whether it's integer or double based on the presence of
// '.', 'e', or 'E'.  Parse accordingly.
// -----------------------------------------------------------------------

ParseResult Parser::ParseNumber()
{
  std::size_t startPosition = this->position;
  int startLine = this->line;
  int startColumn = this->column;

  bool isFloatingPoint = false;

  // Optional leading minus
  if (this->Peek() == '-') {
    this->Advance();
  }

  // Integer part
  if (this->IsAtEnd() || !std::isdigit(static_cast<unsigned char>(this->Peek()))) {
    return this->MakeError("Expected digit after '-'");
  }

  if (this->Peek() == '0') {
    this->Advance();
    // Leading zero must not be followed by another digit (RFC 8259)
    if (!this->IsAtEnd() && std::isdigit(static_cast<unsigned char>(this->Peek()))) {
      return this->MakeError("Leading zeros are not allowed in numbers");
    }
  }
  else {
    while (!this->IsAtEnd() && std::isdigit(static_cast<unsigned char>(this->Peek()))) {
      this->Advance();
    }
  }

  // Fractional part
  if (!this->IsAtEnd() && this->Peek() == '.') {
    isFloatingPoint = true;
    this->Advance();
    if (this->IsAtEnd() || !std::isdigit(static_cast<unsigned char>(this->Peek()))) {
      return this->MakeError("Expected digit after decimal point");
    }
    while (!this->IsAtEnd() && std::isdigit(static_cast<unsigned char>(this->Peek()))) {
      this->Advance();
    }
  }

  // Exponent part
  if (!this->IsAtEnd() && (this->Peek() == 'e' || this->Peek() == 'E')) {
    isFloatingPoint = true;
    this->Advance();
    if (!this->IsAtEnd() && (this->Peek() == '+' || this->Peek() == '-')) {
      this->Advance();
    }
    if (this->IsAtEnd() || !std::isdigit(static_cast<unsigned char>(this->Peek()))) {
      return this->MakeError("Expected digit in exponent");
    }
    while (!this->IsAtEnd() && std::isdigit(static_cast<unsigned char>(this->Peek()))) {
      this->Advance();
    }
  }

  std::string_view numberText = this->input.substr(startPosition, this->position - startPosition);

  if (isFloatingPoint) {
    // Parse as double
    double doubleValue = 0.0;
    auto [endPointer, errorCode] =
        std::from_chars(numberText.data(), numberText.data() + numberText.size(), doubleValue);
    if (errorCode != std::errc()) {
      return ParseResult{
          false, Value(),
          ParseError{"Failed to parse floating-point number", startLine, startColumn}
      };
    }
    return ParseResult{true, Value(doubleValue), {}};
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
        return ParseResult{
            false, Value(), ParseError{"Failed to parse number (overflow)", startLine, startColumn}
        };
      }
      return ParseResult{true, Value(fallbackDouble), {}};
    }
    if (errorCode != std::errc()) {
      return ParseResult{
          false, Value(), ParseError{"Failed to parse integer", startLine, startColumn}
      };
    }
    return ParseResult{true, Value(integerValue), {}};
  }
}

// -----------------------------------------------------------------------
// Strings — handles all RFC 8259 escape sequences including \uXXXX.
// -----------------------------------------------------------------------

int Parser::HexDigitValue(char character)
{
  if (character >= '0' && character <= '9')
    return character - '0';
  if (character >= 'a' && character <= 'f')
    return 10 + (character - 'a');
  if (character >= 'A' && character <= 'F')
    return 10 + (character - 'A');
  return -1;
}

bool Parser::ParseFourHexDigits(uint16_t& outCodeUnit)
{
  uint16_t codeUnit = 0;
  for (int i = 0; i < 4; ++i) {
    if (this->IsAtEnd()) {
      return false;
    }
    int digitValue = HexDigitValue(this->Peek());
    if (digitValue < 0) {
      return false;
    }
    codeUnit = static_cast<uint16_t>((codeUnit << 4) | static_cast<uint16_t>(digitValue));
    this->Advance();
  }
  outCodeUnit = codeUnit;
  return true;
}

// Encode a Unicode code point as UTF-8 and append to the output string.
static void EncodeUtf8(std::string& output, uint32_t codePoint)
{
  if (codePoint <= 0x7F) {
    output.push_back(static_cast<char>(codePoint));
  }
  else if (codePoint <= 0x7FF) {
    output.push_back(static_cast<char>(0xC0 | (codePoint >> 6)));
    output.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
  }
  else if (codePoint <= 0xFFFF) {
    output.push_back(static_cast<char>(0xE0 | (codePoint >> 12)));
    output.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
    output.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
  }
  else if (codePoint <= 0x10FFFF) {
    output.push_back(static_cast<char>(0xF0 | (codePoint >> 18)));
    output.push_back(static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F)));
    output.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
    output.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
  }
}

ParseResult Parser::ParseString()
{
  if (this->IsAtEnd() || this->Peek() != '"') {
    return this->MakeError("Expected '\"' at start of string");
  }
  this->Advance(); // consume opening quote

  std::string result;
  while (true) {
    if (this->IsAtEnd()) {
      return this->MakeError("Unterminated string literal");
    }

    char current = this->Peek();

    if (current == '"') {
      this->Advance(); // consume closing quote
      return ParseResult{true, Value(std::move(result)), {}};
    }

    if (static_cast<unsigned char>(current) < 0x20) {
      return this->MakeError("Unescaped control character in string");
    }

    if (current == '\\') {
      this->Advance(); // consume backslash
      if (this->IsAtEnd()) {
        return this->MakeError("Unterminated escape sequence");
      }
      char escapeCharacter = this->Advance();
      switch (escapeCharacter) {
      case '"':
        result.push_back('"');
        break;
      case '\\':
        result.push_back('\\');
        break;
      case '/':
        result.push_back('/');
        break;
      case 'b':
        result.push_back('\b');
        break;
      case 'f':
        result.push_back('\f');
        break;
      case 'n':
        result.push_back('\n');
        break;
      case 'r':
        result.push_back('\r');
        break;
      case 't':
        result.push_back('\t');
        break;
      case 'u': {
        uint16_t highCodeUnit = 0;
        if (!ParseFourHexDigits(highCodeUnit)) {
          return this->MakeError("Invalid \\u escape sequence");
        }
        // Check for surrogate pair
        if (highCodeUnit >= 0xD800 && highCodeUnit <= 0xDBFF) {
          // High surrogate — expect \uXXXX low surrogate
          if (this->IsAtEnd() || this->Peek() != '\\') {
            return this->MakeError("Expected low surrogate after high surrogate");
          }
          this->Advance();
          if (this->IsAtEnd() || this->Peek() != 'u') {
            return this->MakeError("Expected \\u for low surrogate");
          }
          this->Advance();
          uint16_t lowCodeUnit = 0;
          if (!ParseFourHexDigits(lowCodeUnit)) {
            return this->MakeError("Invalid low surrogate \\u escape");
          }
          if (lowCodeUnit < 0xDC00 || lowCodeUnit > 0xDFFF) {
            return this->MakeError("Invalid low surrogate value");
          }
          uint32_t fullCodePoint = 0x10000 +
                                   ((static_cast<uint32_t>(highCodeUnit) - 0xD800) << 10) +
                                   (static_cast<uint32_t>(lowCodeUnit) - 0xDC00);
          EncodeUtf8(result, fullCodePoint);
        }
        else if (highCodeUnit >= 0xDC00 && highCodeUnit <= 0xDFFF) {
          return this->MakeError("Unexpected low surrogate without preceding high surrogate");
        }
        else {
          EncodeUtf8(result, highCodeUnit);
        }
        break;
      }
      default:
        return this->MakeError(std::string("Invalid escape sequence '\\") + escapeCharacter + "'");
      }
    }
    else {
      result.push_back(this->Advance());
    }
  }
}

ParseResult Parser::ParseArray()
{
  this->Advance(); // consume '['
  SkipWhitespace();

  Value arrayValue = Value::MakeArray();

  if (!this->IsAtEnd() && this->Peek() == ']') {
    this->Advance();
    return ParseResult{true, std::move(arrayValue), {}};
  }

  while (true) {
    this->SkipWhitespace();

    auto elementResult = this->ParseValue();
    if (!elementResult.succeeded) {
      return elementResult;
    }
    arrayValue.PushBack(std::move(elementResult.value));

    this->SkipWhitespace();

    if (this->IsAtEnd()) {
      return this->MakeError("Unterminated array");
    }

    if (this->Peek() == ']') {
      this->Advance();
      return ParseResult{true, std::move(arrayValue), {}};
    }

    if (this->Peek() != ',') {
      return this->MakeError(
          std::string("Expected ',' or ']' in array, got '") + this->Peek() + "'"
      );
    }
    this->Advance(); // consume ','

    // RFC 8259: no trailing commas
    this->SkipWhitespace();
    if (!this->IsAtEnd() && this->Peek() == ']') {
      return this->MakeError("Trailing comma in array");
    }
  }
}

ParseResult Parser::ParseObject()
{
  this->Advance(); // consume '{'
  this->SkipWhitespace();

  Value objectValue = Value::MakeObject();

  if (!this->IsAtEnd() && this->Peek() == '}') {
    this->Advance();
    return ParseResult{true, std::move(objectValue), {}};
  }

  while (true) {
    this->SkipWhitespace();

    if (this->IsAtEnd() || this->Peek() != '"') {
      return this->MakeError("Expected string key in object");
    }

    auto keyResult = this->ParseString();
    if (!keyResult.succeeded) {
      return keyResult;
    }
    std::string key = keyResult.value.AsString();

    this->SkipWhitespace();

    if (this->IsAtEnd() || this->Peek() != ':') {
      return this->MakeError("Expected ':' after object key");
    }
    this->Advance(); // consume ':'

    this->SkipWhitespace();

    auto valueResult = this->ParseValue();
    if (!valueResult.succeeded) {
      return valueResult;
    }

    objectValue.Insert(std::move(key), std::move(valueResult.value));

    this->SkipWhitespace();

    if (this->IsAtEnd()) {
      return this->MakeError("Unterminated object");
    }

    if (this->Peek() == '}') {
      this->Advance();
      return ParseResult{true, std::move(objectValue), {}};
    }

    if (this->Peek() != ',') {
      return this->MakeError(
          std::string("Expected ',' or '}' in object, got '") + this->Peek() + "'"
      );
    }
    this->Advance(); // consume ','

    // RFC 8259: no trailing commas
    this->SkipWhitespace();
    if (!this->IsAtEnd() && this->Peek() == '}') {
      return this->MakeError("Trailing comma in object");
    }
  }
}

ParseResult Parser::ParseValue()
{
  this->SkipWhitespace();

  if (this->IsAtEnd()) {
    return this->MakeError("Unexpected end of input");
  }

  char current = this->Peek();

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
    return this->MakeError(std::string("Unexpected character '") + current + "'");
  }
}
} // namespace Amanuensis
