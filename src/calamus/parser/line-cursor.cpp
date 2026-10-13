#include "calamus/parser/line-cursor.hpp"
#include "calamus/syntax.hpp"

#include <cstddef>
#include <utility>

namespace amanuensis::calamus {

LineCursor::LineCursor(const SourceLine& line)
    : core::Cursor(line.text, line.number)
    , text(line.text)
{
}

std::string_view LineCursor::Rest() const
{
  return this->text.substr(this->Position());
}

void LineCursor::SkipSpaces()
{
  while (this->Peek() == ' ') {
    this->Advance();
  }
}

std::string_view LineCursor::ReadIdentifier()
{
  std::size_t start = this->Position();
  while (IsIdentifierCharacter(this->Peek())) {
    this->Advance();
  }
  return this->Slice(start);
}

core::ParseError LineCursor::ErrorAt(std::string message, int column) const
{
  return core::ParseError{std::move(message), this->Line(), column};
}

} // namespace amanuensis::calamus
