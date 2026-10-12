#include "json/parser/parser.hpp"
#include "json/parser/whitespace.hpp"

#include <string>

namespace amanuensis::json {

Parser::Parser(std::string_view input)
    : cursor(input)
{
}

core::ParseResult Parser::Parse()
{
  auto result = this->values.Parse(this->cursor);
  if (!result.succeeded) {
    return result;
  }
  SkipWhitespace(this->cursor);
  if (!this->cursor.IsAtEnd()) {
    return this->cursor.MakeError(
        std::string("Unexpected content after JSON value: '") + this->cursor.Peek() + "'"
    );
  }
  return result;
}

} // namespace amanuensis::json
