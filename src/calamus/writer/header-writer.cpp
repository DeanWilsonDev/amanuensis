#include "calamus/writer/header-writer.hpp"
#include "calamus/syntax.hpp"
#include "core/quoted-string/quoted-string.hpp"

#include <charconv>
#include <cstddef>

namespace amanuensis::calamus {

static void AppendHex(std::string& output, std::uint64_t value)
{
  char digits[16];
  auto [endPointer, errorCode] = std::to_chars(digits, digits + sizeof(digits), value, 16);
  output.append(sizeof(digits) - static_cast<std::size_t>(endPointer - digits), '0');
  output.append(digits, endPointer);
}

std::optional<std::string> ValidateHeader(const Header& header)
{
  if (header.version < 1) {
    return "@version must be at least 1";
  }
  if (!IsTypeName(header.type)) {
    return "@type must be a name: a letter or '_', then letters, digits, '_', '.' or ':'";
  }
  return std::nullopt;
}

std::string WriteHeader(const Header& header, std::uint64_t baseHash)
{
  std::string text = "@version " + std::to_string(header.version) + "\n@type " + header.type;
  if (header.id) {
    text.append("\n@id ");
    AppendHex(text, *header.id);
  }
  if (header.base) {
    text.append("\n@base ");
    core::AppendQuotedString(text, *header.base);
  }
  text.append("\n@base_hash ");
  AppendHex(text, baseHash);
  text.push_back('\n');
  return text;
}

} // namespace amanuensis::calamus
