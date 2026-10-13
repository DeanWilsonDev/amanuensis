#pragma once

#include "amanuensis/core/parse-result.hpp"
#include "core/cursor.hpp"
#include "json/parser/value-parser.hpp"

#include <string_view>

namespace amanuensis::json {

class Parser {
public:
  explicit Parser(std::string_view input);

  core::ParseResult Parse();

private:
  core::Cursor cursor;
  ValueParser values;
};

} // namespace amanuensis::json
