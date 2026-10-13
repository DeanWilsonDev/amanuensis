#pragma once

#include <string_view>

namespace amanuensis::calamus {

inline bool IsIdentifierStart(char character)
{
  return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') ||
         character == '_';
}

inline bool IsIdentifierCharacter(char character)
{
  return IsIdentifierStart(character) || (character >= '0' && character <= '9');
}

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
