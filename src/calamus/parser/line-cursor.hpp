#pragma once

#include "amanuensis/core/parse-error.hpp"
#include "calamus/parser/source-text.hpp"
#include "core/cursor.hpp"

#include <string>
#include <string_view>

namespace amanuensis::calamus {

class LineCursor : public core::Cursor {
public:
  explicit LineCursor(const SourceLine& line);

  std::string_view Rest() const;
  void SkipSpaces();
  std::string_view ReadIdentifier();

  core::ParseError ErrorAt(std::string message, int column) const;

private:
  std::string_view text;
};

} // namespace amanuensis::calamus
