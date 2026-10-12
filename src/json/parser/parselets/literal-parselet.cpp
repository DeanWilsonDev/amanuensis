#include "json/parser/parselets/literal-parselet.hpp"

#include <string>

namespace amanuensis::json {

core::ParseResult LiteralParselet::Parse(core::Cursor& cursor, const ValueParser&) const
{
  for (char expected : this->text) {
    if (cursor.IsAtEnd() || cursor.Peek() != expected) {
      return cursor.MakeError("Invalid literal, expected '" + std::string(this->text) + "'");
    }
    cursor.Advance();
  }
  return core::ParseResult{true, this->value, {}};
}

} // namespace amanuensis::json
