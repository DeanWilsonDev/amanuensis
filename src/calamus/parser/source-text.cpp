#include "calamus/parser/source-text.hpp"

#include <cstddef>

namespace amanuensis::calamus {

MaybeError SplitLines(std::string_view input, SourceText& source)
{
  constexpr std::string_view byteOrderMark = "\xEF\xBB\xBF";
  if (input.starts_with(byteOrderMark)) {
    input.remove_prefix(byteOrderMark.size());
  }

  int number = 1;
  while (true) {
    std::size_t lineEnd = input.find('\n');
    std::string_view text = input.substr(0, lineEnd);
    if (lineEnd != std::string_view::npos && text.ends_with('\r')) {
      text.remove_suffix(1);
    }

    std::size_t carriageReturn = text.find('\r');
    if (carriageReturn != std::string_view::npos) {
      return core::ParseError{
          "A carriage return must be followed by a line feed", number,
          static_cast<int>(carriageReturn) + 1
      };
    }

    source.endLine = number;
    source.endColumn = static_cast<int>(text.size()) + 1;

    while (!text.empty() && (text.back() == ' ' || text.back() == '\t')) {
      text.remove_suffix(1);
    }
    if (!text.empty()) {
      source.lines.push_back(SourceLine{text, number});
    }

    if (lineEnd == std::string_view::npos) {
      return std::nullopt;
    }
    input.remove_prefix(lineEnd + 1);
    ++number;
  }
}

} // namespace amanuensis::calamus
