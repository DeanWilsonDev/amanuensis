#include "calamus/writer/key-writer.hpp"
#include "calamus/syntax.hpp"
#include "core/quoted-string/quoted-string.hpp"

namespace amanuensis::calamus {

void AppendKey(std::string& output, const std::string& key)
{
  if (IsBareKey(key)) {
    output.append(key);
  }
  else {
    core::AppendQuotedString(output, key);
  }
}

std::string FormatPath(const core::KeyPath& path)
{
  std::string formatted;
  for (const std::string& key : path) {
    if (!formatted.empty()) {
      formatted.push_back('.');
    }
    AppendKey(formatted, key);
  }
  return formatted;
}

} // namespace amanuensis::calamus
