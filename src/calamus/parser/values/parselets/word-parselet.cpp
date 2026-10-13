#include "calamus/parser/values/parselets/word-parselet.hpp"

#include <string>
#include <string_view>

namespace amanuensis::calamus {

core::ParseResult WordParselet::Parse(LineCursor& cursor, const ValueParser&) const
{
  int column = cursor.Column();
  std::string_view word = cursor.ReadIdentifier();
  if (word == "null") {
    return core::ParseResult{true, core::Value(), {}};
  }
  if (word == "true") {
    return core::ParseResult{true, core::Value(true), {}};
  }
  if (word == "false") {
    return core::ParseResult{true, core::Value(false), {}};
  }
  return core::ParseResult{
      false, core::Value(),
      cursor.ErrorAt("Strings must be quoted, as in \"" + std::string(word) + "\"", column)
  };
}

} // namespace amanuensis::calamus
