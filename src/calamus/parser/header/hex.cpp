#include "calamus/parser/header/hex.hpp"

#include <charconv>

namespace amanuensis::calamus {

static bool IsHexDigit(char character)
{
  return (character >= '0' && character <= '9') || (character >= 'a' && character <= 'f') ||
         (character >= 'A' && character <= 'F');
}

std::optional<std::uint64_t> ParseHex(std::string_view text, std::size_t maxDigits)
{
  if (text.empty() || text.size() > maxDigits) {
    return std::nullopt;
  }
  for (char character : text) {
    if (!IsHexDigit(character)) {
      return std::nullopt;
    }
  }
  std::uint64_t value = 0;
  std::from_chars(text.data(), text.data() + text.size(), value, 16);
  return value;
}

} // namespace amanuensis::calamus
