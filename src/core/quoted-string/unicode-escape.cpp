#include "core/quoted-string/unicode-escape.hpp"
#include "core/quoted-string/utf8.hpp"

#include <cstdint>

namespace amanuensis::core {

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

static bool ReadFourHexDigits(Cursor& cursor, std::uint16_t& outCodeUnit)
{
  std::uint16_t codeUnit = 0;
  for (int i = 0; i < 4; ++i) {
    if (cursor.IsAtEnd()) {
      return false;
    }
    int digitValue = HexDigitValue(cursor.Peek());
    if (digitValue < 0) {
      return false;
    }
    codeUnit = static_cast<std::uint16_t>((codeUnit << 4) | static_cast<std::uint16_t>(digitValue));
    cursor.Advance();
  }
  outCodeUnit = codeUnit;
  return true;
}

static bool IsHighSurrogate(std::uint16_t codeUnit)
{
  return codeUnit >= 0xD800 && codeUnit <= 0xDBFF;
}

static bool IsLowSurrogate(std::uint16_t codeUnit)
{
  return codeUnit >= 0xDC00 && codeUnit <= 0xDFFF;
}

static std::uint32_t CombineSurrogates(std::uint16_t high, std::uint16_t low)
{
  return 0x10000 + ((static_cast<std::uint32_t>(high) - 0xD800) << 10) +
         (static_cast<std::uint32_t>(low) - 0xDC00);
}

static std::optional<ParseError> ReadLowSurrogate(Cursor& cursor, std::uint16_t& low)
{
  if (cursor.IsAtEnd() || cursor.Peek() != '\\') {
    return cursor.ErrorHere("Expected low surrogate after high surrogate");
  }
  cursor.Advance();
  if (cursor.IsAtEnd() || cursor.Peek() != 'u') {
    return cursor.ErrorHere("Expected \\u for low surrogate");
  }
  cursor.Advance();
  if (!ReadFourHexDigits(cursor, low)) {
    return cursor.ErrorHere("Invalid low surrogate \\u escape");
  }
  if (!IsLowSurrogate(low)) {
    return cursor.ErrorHere("Invalid low surrogate value");
  }
  return std::nullopt;
}

std::optional<ParseError> ReadUnicodeEscape(Cursor& cursor, std::string& output)
{
  std::uint16_t codeUnit = 0;
  if (!ReadFourHexDigits(cursor, codeUnit)) {
    return cursor.ErrorHere("Invalid \\u escape sequence");
  }
  if (IsLowSurrogate(codeUnit)) {
    return cursor.ErrorHere("Unexpected low surrogate without preceding high surrogate");
  }
  if (!IsHighSurrogate(codeUnit)) {
    AppendUtf8(output, codeUnit);
    return std::nullopt;
  }

  std::uint16_t low = 0;
  if (auto error = ReadLowSurrogate(cursor, low)) {
    return error;
  }
  AppendUtf8(output, CombineSurrogates(codeUnit, low));
  return std::nullopt;
}

} // namespace amanuensis::core
