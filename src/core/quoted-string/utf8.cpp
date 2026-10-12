#include "core/quoted-string/utf8.hpp"

namespace amanuensis::core {

void AppendUtf8(std::string& output, std::uint32_t codePoint)
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

} // namespace amanuensis::core
