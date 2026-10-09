#include "core/quoted-string.hpp"

#include <cstdint>
#include <cstdio>

namespace amanuensis::core {

// -----------------------------------------------------------------------
// Reading — handles all RFC 8259 escape sequences including \uXXXX.
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

static bool ParseFourHexDigits(Cursor& cursor, uint16_t& outCodeUnit)
{
  uint16_t codeUnit = 0;
  for (int i = 0; i < 4; ++i) {
    if (cursor.IsAtEnd()) {
      return false;
    }
    int digitValue = HexDigitValue(cursor.Peek());
    if (digitValue < 0) {
      return false;
    }
    codeUnit = static_cast<uint16_t>((codeUnit << 4) | static_cast<uint16_t>(digitValue));
    cursor.Advance();
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

ParseResult ParseQuotedString(Cursor& cursor)
{
  if (cursor.IsAtEnd() || cursor.Peek() != '"') {
    return cursor.MakeError("Expected '\"' at start of string");
  }
  cursor.Advance(); // consume opening quote

  std::string result;
  while (true) {
    if (cursor.IsAtEnd()) {
      return cursor.MakeError("Unterminated string literal");
    }

    char current = cursor.Peek();

    if (current == '"') {
      cursor.Advance(); // consume closing quote
      return ParseResult{true, Value(std::move(result)), {}};
    }

    if (static_cast<unsigned char>(current) < 0x20) {
      return cursor.MakeError("Unescaped control character in string");
    }

    if (current == '\\') {
      cursor.Advance(); // consume backslash
      if (cursor.IsAtEnd()) {
        return cursor.MakeError("Unterminated escape sequence");
      }
      char escapeCharacter = cursor.Advance();
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
        if (!ParseFourHexDigits(cursor, highCodeUnit)) {
          return cursor.MakeError("Invalid \\u escape sequence");
        }
        // Check for surrogate pair
        if (highCodeUnit >= 0xD800 && highCodeUnit <= 0xDBFF) {
          // High surrogate — expect \uXXXX low surrogate
          if (cursor.IsAtEnd() || cursor.Peek() != '\\') {
            return cursor.MakeError("Expected low surrogate after high surrogate");
          }
          cursor.Advance();
          if (cursor.IsAtEnd() || cursor.Peek() != 'u') {
            return cursor.MakeError("Expected \\u for low surrogate");
          }
          cursor.Advance();
          uint16_t lowCodeUnit = 0;
          if (!ParseFourHexDigits(cursor, lowCodeUnit)) {
            return cursor.MakeError("Invalid low surrogate \\u escape");
          }
          if (lowCodeUnit < 0xDC00 || lowCodeUnit > 0xDFFF) {
            return cursor.MakeError("Invalid low surrogate value");
          }
          uint32_t fullCodePoint = 0x10000 +
                                   ((static_cast<uint32_t>(highCodeUnit) - 0xD800) << 10) +
                                   (static_cast<uint32_t>(lowCodeUnit) - 0xDC00);
          EncodeUtf8(result, fullCodePoint);
        }
        else if (highCodeUnit >= 0xDC00 && highCodeUnit <= 0xDFFF) {
          return cursor.MakeError("Unexpected low surrogate without preceding high surrogate");
        }
        else {
          EncodeUtf8(result, highCodeUnit);
        }
        break;
      }
      default:
        return cursor.MakeError(std::string("Invalid escape sequence '\\") + escapeCharacter + "'");
      }
    }
    else {
      result.push_back(cursor.Advance());
    }
  }
}

// -----------------------------------------------------------------------
// Writing
// -----------------------------------------------------------------------

void AppendQuotedString(std::string& output, std::string_view text)
{
  output.push_back('"');
  for (char character : text) {
    switch (character) {
    case '"':
      output.append("\\\"");
      break;
    case '\\':
      output.append("\\\\");
      break;
    case '\b':
      output.append("\\b");
      break;
    case '\f':
      output.append("\\f");
      break;
    case '\n':
      output.append("\\n");
      break;
    case '\r':
      output.append("\\r");
      break;
    case '\t':
      output.append("\\t");
      break;
    default:
      if (static_cast<unsigned char>(character) < 0x20) {
        // Control characters — emit as \u00XX
        char hexBuffer[8];
        std::snprintf(
            hexBuffer, sizeof(hexBuffer), "\\u%04x", static_cast<unsigned char>(character)
        );
        output.append(hexBuffer);
      }
      else {
        output.push_back(character);
      }
      break;
    }
  }
  output.push_back('"');
}

} // namespace amanuensis::core
