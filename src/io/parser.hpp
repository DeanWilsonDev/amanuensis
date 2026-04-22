#pragma once

#include <amanuensis/io/parser-result.hpp>
#include "amanuensis/value.hpp"
#include <string>

namespace Amanuensis {

class Parser {
public:
  Parser(std::string_view input);
  ~Parser();

  ParseResult Parse();
  bool IsAtEnd() const;
  char Peek() const;
  char Advance();
  ParseResult MakeError(const std::string& message) const;

  void SkipWhitespace();
  ParseResult ParseNull();
  ParseResult ParseString();
  ParseResult ParseNumber();
  ParseResult ParseArray();
  ParseResult ParseObject();
  ParseResult ParseTrue();
  ParseResult ParseFalse();
  ParseResult ParseValue();
  void SetInput(std::string_view input);

private:
  static int HexDigitValue(char character);
  bool ParseFourHexDigits(uint16_t& outCodeUnit);

  std::string_view input;
  std::size_t cursor = 0;
  int line = 1;
  int column = 1;
};

} // namespace Amanuensis
