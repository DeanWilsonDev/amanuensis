#include "core/quoted-string/quoted-string.hpp"

#include <cstdio>

namespace amanuensis::core {

static void AppendEscaped(std::string& output, char character)
{
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
      char hexBuffer[8];
      std::snprintf(hexBuffer, sizeof(hexBuffer), "\\u%04x", static_cast<unsigned char>(character));
      output.append(hexBuffer);
    }
    else {
      output.push_back(character);
    }
    break;
  }
}

void AppendQuotedString(std::string& output, std::string_view text)
{
  output.push_back('"');
  for (char character : text) {
    AppendEscaped(output, character);
  }
  output.push_back('"');
}

} // namespace amanuensis::core
