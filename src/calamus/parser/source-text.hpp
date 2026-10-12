#pragma once

#include "calamus/parser/maybe-error.hpp"

#include <string_view>
#include <vector>

namespace amanuensis::calamus {

// One non-blank line of the input, without its line ending or trailing
// whitespace.
struct SourceLine {
  std::string_view text;
  int number;
};

struct SourceText {
  std::vector<SourceLine> lines;
  int endLine = 1;
  int endColumn = 1;
};

MaybeError SplitLines(std::string_view input, SourceText& source);

} // namespace amanuensis::calamus
