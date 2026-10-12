#include "core/number-format.hpp"

#include <cctype>
#include <charconv>
#include <string_view>

namespace amanuensis::core {

// -----------------------------------------------------------------------
// Reading
//
// Strategy: identify the span of characters that form the number literal,
// then decide whether it's integer or double based on the presence of
// '.', 'e', or 'E'.  Parse accordingly.
// -----------------------------------------------------------------------

static bool IsDigit(char character)
{
  return std::isdigit(static_cast<unsigned char>(character)) != 0;
}

ParseResult ParseNumber(Cursor& cursor)
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
  if (cursor.IsAtEnd() || !IsDigit(cursor.Peek())) {
    return cursor.MakeError("Expected digit after '-'");
  }

  if (cursor.Peek() == '0') {
    cursor.Advance();
    // Leading zero must not be followed by another digit (RFC 8259)
    if (!cursor.IsAtEnd() && IsDigit(cursor.Peek())) {
      return cursor.MakeError("Leading zeros are not allowed in numbers");
    }
  }
  else {
    while (!cursor.IsAtEnd() && IsDigit(cursor.Peek())) {
      cursor.Advance();
    }
  }

  // Fractional part
  if (!cursor.IsAtEnd() && cursor.Peek() == '.') {
    isFloatingPoint = true;
    cursor.Advance();
    if (cursor.IsAtEnd() || !IsDigit(cursor.Peek())) {
      return cursor.MakeError("Expected digit after decimal point");
    }
    while (!cursor.IsAtEnd() && IsDigit(cursor.Peek())) {
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
    if (cursor.IsAtEnd() || !IsDigit(cursor.Peek())) {
      return cursor.MakeError("Expected digit in exponent");
    }
    while (!cursor.IsAtEnd() && IsDigit(cursor.Peek())) {
      cursor.Advance();
    }
  }

  std::string_view numberText = cursor.Slice(startPosition);

  if (isFloatingPoint) {
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

  // Try integer first; fall back to double on overflow
  long long integerValue = 0;
  auto [endPointer, errorCode] =
      std::from_chars(numberText.data(), numberText.data() + numberText.size(), integerValue);
  if (errorCode == std::errc::result_out_of_range) {
    // Overflow — fall back to double silently, as per design doc
    double fallbackDouble = 0.0;
    auto [doubleEnd, doubleError] =
        std::from_chars(numberText.data(), numberText.data() + numberText.size(), fallbackDouble);
    if (doubleError != std::errc()) {
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

// -----------------------------------------------------------------------
// Writing
// -----------------------------------------------------------------------

void AppendInteger(std::string& output, long long value)
{
  char formatBuffer[32];
  auto [endPointer, errorCode] =
      std::to_chars(formatBuffer, formatBuffer + sizeof(formatBuffer), value);
  output.append(formatBuffer, static_cast<std::size_t>(endPointer - formatBuffer));
}

template <typename TFloatingPoint>
static void AppendShortest(std::string& output, TFloatingPoint value)
{
  char formatBuffer[64];
  auto [endPointer, errorCode] =
      std::to_chars(formatBuffer, formatBuffer + sizeof(formatBuffer), value);
  std::string_view formatted(formatBuffer, static_cast<std::size_t>(endPointer - formatBuffer));

  output.append(formatted);
  if (formatted.find_first_of(".eE") == std::string_view::npos) {
    output.append(".0");
  }
}

void AppendDouble(std::string& output, double value)
{
  AppendShortest(output, value);
}

void AppendFloat(std::string& output, float value)
{
  AppendShortest(output, value);
}

} // namespace amanuensis::core
