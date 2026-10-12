#pragma once

#include <string_view>

namespace amanuensis::calamus {

// Character rules the reader and writer share, so the writer never writes a
// key or header value that the reader would reject.

inline bool IsIdentifierStart(char character)
{
  return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') ||
         character == '_';
}

inline bool IsIdentifierCharacter(char character)
{
  return IsIdentifierStart(character) || (character >= '0' && character <= '9');
}

// A key that can be written without quotes: [A-Za-z_][A-Za-z0-9_]*.
inline bool IsBareKey(std::string_view text)
{
  if (text.empty() || !IsIdentifierStart(text.front())) {
    return false;
  }
  for (char character : text) {
    if (!IsIdentifierCharacter(character)) {
      return false;
    }
  }
  return true;
}

// An @type value: a letter or '_', then letters, digits, '_', '.' or ':'.
inline bool IsTypeName(std::string_view text)
{
  if (text.empty() || !IsIdentifierStart(text.front())) {
    return false;
  }
  for (char character : text) {
    if (!IsIdentifierCharacter(character) && character != '.' && character != ':') {
      return false;
    }
  }
  return true;
}

} // namespace amanuensis::calamus
