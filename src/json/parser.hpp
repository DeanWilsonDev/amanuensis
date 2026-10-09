#pragma once

#include <amanuensis/core/parse-result.hpp>
#include "amanuensis/core/value.hpp"
#include <string>
#include <cstdint>

namespace amanuensis::json {

class Parser {
public:
  Parser(std::string_view input);
  ~Parser();

  core::ParseResult Parse();
  bool IsAtEnd() const;
  char Peek() const;
  char Advance();
  core::ParseResult MakeError(const std::string& message) const;

  void SkipWhitespace();
  core::ParseResult ParseNull();
  core::ParseResult ParseString();
  core::ParseResult ParseNumber();
  core::ParseResult ParseArray();
  core::ParseResult ParseObject();
  core::ParseResult ParseTrue();
  core::ParseResult ParseFalse();
  core::ParseResult ParseValue();
  void SetInput(std::string_view input);

private:
  static int HexDigitValue(char character);
  bool ParseFourHexDigits(uint16_t& outCodeUnit);

  std::string_view input;
  std::size_t cursor = 0;
  int line = 1;
  int column = 1;
};

} // namespace amanuensis::json
