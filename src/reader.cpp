#include "amanuensis/reader.hpp"

#include <cerrno>
#include <charconv>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace Amanuensis {

// -----------------------------------------------------------------------
// Lexer + recursive-descent parser with explicit line/column tracking.
//
// The parser consumes a string_view and maintains a cursor (position),
// a line number, and a column number.  Every character advance updates
// line/column so that error messages are precise.
// -----------------------------------------------------------------------

namespace {

struct ParserState {
  std::string_view input;
  std::size_t position = 0;
  int line = 1;
  int column = 1;

  bool IsAtEnd() const { return position >= input.size(); }

  char Peek() const
  {
    if (IsAtEnd()) {
      return '\0';
    }
    return input[position];
  }

  char Advance()
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

  ParseResult MakeError(const std::string& message) const
  {
    return ParseResult{false, Value(), ParseError{message, line, column}};
  }
};

// -----------------------------------------------------------------------
// Forward declarations
// -----------------------------------------------------------------------

ParseResult ParseValue(ParserState& state);

// -----------------------------------------------------------------------
// Whitespace
// -----------------------------------------------------------------------

void SkipWhitespace(ParserState& state)
{
  while (!state.IsAtEnd()) {
    char current = state.Peek();
    if (current == ' ' || current == '\t' || current == '\n' || current == '\r') {
      state.Advance();
    }
    else {
      break;
    }
  }
}

// -----------------------------------------------------------------------
// Literal keywords: null, true, false
// -----------------------------------------------------------------------

ParseResult ParseNull(ParserState& state)
{
  const char* expected = "null";
  for (int i = 0; i < 4; ++i) {
    if (state.IsAtEnd() || state.Peek() != expected[i]) {
      return state.MakeError("Invalid literal, expected 'null'");
    }
    state.Advance();
  }
  return ParseResult{true, Value(), {}};
}

ParseResult ParseTrue(ParserState& state)
{
  const char* expected = "true";
  for (int i = 0; i < 4; ++i) {
    if (state.IsAtEnd() || state.Peek() != expected[i]) {
      return state.MakeError("Invalid literal, expected 'true'");
    }
    state.Advance();
  }
  return ParseResult{true, Value(true), {}};
}

ParseResult ParseFalse(ParserState& state)
{
  const char* expected = "false";
  for (int i = 0; i < 5; ++i) {
    if (state.IsAtEnd() || state.Peek() != expected[i]) {
      return state.MakeError("Invalid literal, expected 'false'");
    }
    state.Advance();
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

ParseResult ParseNumber(ParserState& state)
{
  std::size_t startPosition = state.position;
  int startLine = state.line;
  int startColumn = state.column;

  bool isFloatingPoint = false;

  // Optional leading minus
  if (state.Peek() == '-') {
    state.Advance();
  }

  // Integer part
  if (state.IsAtEnd() || !std::isdigit(static_cast<unsigned char>(state.Peek()))) {
    return state.MakeError("Expected digit after '-'");
  }

  if (state.Peek() == '0') {
    state.Advance();
    // Leading zero must not be followed by another digit (RFC 8259)
    if (!state.IsAtEnd() && std::isdigit(static_cast<unsigned char>(state.Peek()))) {
      return state.MakeError("Leading zeros are not allowed in numbers");
    }
  }
  else {
    while (!state.IsAtEnd() && std::isdigit(static_cast<unsigned char>(state.Peek()))) {
      state.Advance();
    }
  }

  // Fractional part
  if (!state.IsAtEnd() && state.Peek() == '.') {
    isFloatingPoint = true;
    state.Advance();
    if (state.IsAtEnd() || !std::isdigit(static_cast<unsigned char>(state.Peek()))) {
      return state.MakeError("Expected digit after decimal point");
    }
    while (!state.IsAtEnd() && std::isdigit(static_cast<unsigned char>(state.Peek()))) {
      state.Advance();
    }
  }

  // Exponent part
  if (!state.IsAtEnd() && (state.Peek() == 'e' || state.Peek() == 'E')) {
    isFloatingPoint = true;
    state.Advance();
    if (!state.IsAtEnd() && (state.Peek() == '+' || state.Peek() == '-')) {
      state.Advance();
    }
    if (state.IsAtEnd() || !std::isdigit(static_cast<unsigned char>(state.Peek()))) {
      return state.MakeError("Expected digit in exponent");
    }
    while (!state.IsAtEnd() && std::isdigit(static_cast<unsigned char>(state.Peek()))) {
      state.Advance();
    }
  }

  std::string_view numberText = state.input.substr(startPosition, state.position - startPosition);

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

static int HexDigitValue(char character)
{
  if (character >= '0' && character <= '9')
    return character - '0';
  if (character >= 'a' && character <= 'f')
    return 10 + (character - 'a');
  if (character >= 'A' && character <= 'F')
    return 10 + (character - 'A');
  return -1;
}

static bool ParseFourHexDigits(ParserState& state, uint16_t& outCodeUnit)
{
  uint16_t codeUnit = 0;
  for (int i = 0; i < 4; ++i) {
    if (state.IsAtEnd()) {
      return false;
    }
    int digitValue = HexDigitValue(state.Peek());
    if (digitValue < 0) {
      return false;
    }
    codeUnit = static_cast<uint16_t>((codeUnit << 4) | static_cast<uint16_t>(digitValue));
    state.Advance();
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

ParseResult ParseString(ParserState& state)
{
  if (state.IsAtEnd() || state.Peek() != '"') {
    return state.MakeError("Expected '\"' at start of string");
  }
  state.Advance(); // consume opening quote

  std::string result;
  while (true) {
    if (state.IsAtEnd()) {
      return state.MakeError("Unterminated string literal");
    }

    char current = state.Peek();

    if (current == '"') {
      state.Advance(); // consume closing quote
      return ParseResult{true, Value(std::move(result)), {}};
    }

    if (static_cast<unsigned char>(current) < 0x20) {
      return state.MakeError("Unescaped control character in string");
    }

    if (current == '\\') {
      state.Advance(); // consume backslash
      if (state.IsAtEnd()) {
        return state.MakeError("Unterminated escape sequence");
      }
      char escapeCharacter = state.Advance();
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
        if (!ParseFourHexDigits(state, highCodeUnit)) {
          return state.MakeError("Invalid \\u escape sequence");
        }
        // Check for surrogate pair
        if (highCodeUnit >= 0xD800 && highCodeUnit <= 0xDBFF) {
          // High surrogate — expect \uXXXX low surrogate
          if (state.IsAtEnd() || state.Peek() != '\\') {
            return state.MakeError("Expected low surrogate after high surrogate");
          }
          state.Advance();
          if (state.IsAtEnd() || state.Peek() != 'u') {
            return state.MakeError("Expected \\u for low surrogate");
          }
          state.Advance();
          uint16_t lowCodeUnit = 0;
          if (!ParseFourHexDigits(state, lowCodeUnit)) {
            return state.MakeError("Invalid low surrogate \\u escape");
          }
          if (lowCodeUnit < 0xDC00 || lowCodeUnit > 0xDFFF) {
            return state.MakeError("Invalid low surrogate value");
          }
          uint32_t fullCodePoint = 0x10000 +
                                   ((static_cast<uint32_t>(highCodeUnit) - 0xD800) << 10) +
                                   (static_cast<uint32_t>(lowCodeUnit) - 0xDC00);
          EncodeUtf8(result, fullCodePoint);
        }
        else if (highCodeUnit >= 0xDC00 && highCodeUnit <= 0xDFFF) {
          return state.MakeError("Unexpected low surrogate without preceding high surrogate");
        }
        else {
          EncodeUtf8(result, highCodeUnit);
        }
        break;
      }
      default:
        return state.MakeError(std::string("Invalid escape sequence '\\") + escapeCharacter + "'");
      }
    }
    else {
      result.push_back(state.Advance());
    }
  }
}

// -----------------------------------------------------------------------
// Arrays
// -----------------------------------------------------------------------

ParseResult ParseArray(ParserState& state)
{
  state.Advance(); // consume '['
  SkipWhitespace(state);

  Value arrayValue = Value::MakeArray();

  if (!state.IsAtEnd() && state.Peek() == ']') {
    state.Advance();
    return ParseResult{true, std::move(arrayValue), {}};
  }

  while (true) {
    SkipWhitespace(state);

    auto elementResult = ParseValue(state);
    if (!elementResult.succeeded) {
      return elementResult;
    }
    arrayValue.PushBack(std::move(elementResult.value));

    SkipWhitespace(state);

    if (state.IsAtEnd()) {
      return state.MakeError("Unterminated array");
    }

    if (state.Peek() == ']') {
      state.Advance();
      return ParseResult{true, std::move(arrayValue), {}};
    }

    if (state.Peek() != ',') {
      return state.MakeError(
          std::string("Expected ',' or ']' in array, got '") + state.Peek() + "'"
      );
    }
    state.Advance(); // consume ','

    // RFC 8259: no trailing commas
    SkipWhitespace(state);
    if (!state.IsAtEnd() && state.Peek() == ']') {
      return state.MakeError("Trailing comma in array");
    }
  }
}

// -----------------------------------------------------------------------
// Objects
// -----------------------------------------------------------------------

ParseResult ParseObject(ParserState& state)
{
  state.Advance(); // consume '{'
  SkipWhitespace(state);

  Value objectValue = Value::MakeObject();

  if (!state.IsAtEnd() && state.Peek() == '}') {
    state.Advance();
    return ParseResult{true, std::move(objectValue), {}};
  }

  while (true) {
    SkipWhitespace(state);

    if (state.IsAtEnd() || state.Peek() != '"') {
      return state.MakeError("Expected string key in object");
    }

    auto keyResult = ParseString(state);
    if (!keyResult.succeeded) {
      return keyResult;
    }
    std::string key = keyResult.value.AsString();

    SkipWhitespace(state);

    if (state.IsAtEnd() || state.Peek() != ':') {
      return state.MakeError("Expected ':' after object key");
    }
    state.Advance(); // consume ':'

    SkipWhitespace(state);

    auto valueResult = ParseValue(state);
    if (!valueResult.succeeded) {
      return valueResult;
    }

    objectValue.Insert(std::move(key), std::move(valueResult.value));

    SkipWhitespace(state);

    if (state.IsAtEnd()) {
      return state.MakeError("Unterminated object");
    }

    if (state.Peek() == '}') {
      state.Advance();
      return ParseResult{true, std::move(objectValue), {}};
    }

    if (state.Peek() != ',') {
      return state.MakeError(
          std::string("Expected ',' or '}' in object, got '") + state.Peek() + "'"
      );
    }
    state.Advance(); // consume ','

    // RFC 8259: no trailing commas
    SkipWhitespace(state);
    if (!state.IsAtEnd() && state.Peek() == '}') {
      return state.MakeError("Trailing comma in object");
    }
  }
}

// -----------------------------------------------------------------------
// Top-level value dispatch
// -----------------------------------------------------------------------

ParseResult ParseValue(ParserState& state)
{
  SkipWhitespace(state);

  if (state.IsAtEnd()) {
    return state.MakeError("Unexpected end of input");
  }

  char current = state.Peek();

  switch (current) {
  case 'n':
    return ParseNull(state);
  case 't':
    return ParseTrue(state);
  case 'f':
    return ParseFalse(state);
  case '"':
    return ParseString(state);
  case '[':
    return ParseArray(state);
  case '{':
    return ParseObject(state);
  default:
    if (current == '-' || (current >= '0' && current <= '9')) {
      return ParseNumber(state);
    }
    return state.MakeError(std::string("Unexpected character '") + current + "'");
  }
}

} // anonymous namespace

// -----------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------

ParseResult Reader::ParseString(std::string_view text)
{
  ParserState state;
  state.input = text;

  auto result = ::Amanuensis::ParseValue(state);
  if (!result.succeeded) {
    return result;
  }

  // Ensure there's no trailing non-whitespace content
  SkipWhitespace(state);
  if (!state.IsAtEnd()) {
    return state.MakeError(
        std::string("Unexpected content after JSON value: '") + state.Peek() + "'"
    );
  }

  return result;
}

ParseResult Reader::ParseFile(const std::filesystem::path& path)
{
  std::ifstream inputFile(path, std::ios::binary);
  if (!inputFile.is_open()) {
    return ParseResult{false, Value(), ParseError{"Could not open file: " + path.string(), 0, 0}};
  }

  std::ostringstream contentStream;
  contentStream << inputFile.rdbuf();
  std::string fileContent = contentStream.str();

  return ParseString(fileContent);
}

} // namespace Amanuensis
