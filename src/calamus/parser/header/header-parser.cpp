#include "calamus/parser/header/header-parser.hpp"
#include "calamus/parser/line-cursor.hpp"

#include <string_view>
#include <utility>

namespace amanuensis::calamus {

MaybeError HeaderParser::ParseLine(const SourceLine& line)
{
  LineCursor cursor(line);
  cursor.Advance();

  std::string name(cursor.ReadIdentifier());
  if (name.empty()) {
    return cursor.ErrorHere("Expected a header name after '@'");
  }

  const HeaderFieldParselet* field = this->fields.Find(name);
  if (field == nullptr) {
    return cursor.ErrorAt("Unknown header line '@" + name + "'", 1);
  }
  if (!this->seen.insert(name).second) {
    return cursor.ErrorAt("Repeated header line '@" + name + "'", 1);
  }

  if (cursor.IsAtEnd()) {
    return cursor.ErrorHere("Expected a value after '@" + name + "'");
  }
  if (cursor.Peek() != ' ') {
    return cursor.ErrorHere("Expected a space after '@" + name + "'");
  }
  cursor.SkipSpaces();
  return field->Parse(cursor, this->header);
}

MaybeError HeaderParser::CheckRequired(const SourceText& source, std::size_t bodyStart) const
{
  for (std::string_view name : this->fields.RequiredNames()) {
    if (this->seen.contains(name)) {
      continue;
    }
    std::string message = "Missing the @" + std::string(name) + " header line";
    if (bodyStart < source.lines.size()) {
      return core::ParseError{message, source.lines[bodyStart].number, 1};
    }
    return core::ParseError{message, source.endLine, source.endColumn};
  }
  return std::nullopt;
}

Header HeaderParser::TakeHeader()
{
  return std::move(this->header);
}

} // namespace amanuensis::calamus
