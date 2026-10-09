#pragma once

#include <amanuensis/core/parse-result.hpp>
#include "core/cursor.hpp"
#include <string>

namespace amanuensis::json {

class Parser {
public:
  Parser(std::string_view input);

  core::ParseResult Parse();

  void SkipWhitespace();
  core::ParseResult ParseNull();
  core::ParseResult ParseString();
  core::ParseResult ParseNumber();
  core::ParseResult ParseArray();
  core::ParseResult ParseObject();
  core::ParseResult ParseTrue();
  core::ParseResult ParseFalse();
  core::ParseResult ParseValue();

private:
  core::Cursor cursor;
};

} // namespace amanuensis::json
