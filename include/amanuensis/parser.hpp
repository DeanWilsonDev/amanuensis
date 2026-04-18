#pragma once

#include "value.hpp"
#include <string>

namespace Amanuensis {

struct ParseError {
  std::string message;
  int line;
  int column;
};

struct ParseResult {
  bool succeeded;
  Value value;
  ParseError error;
};

class Parser {
public:
  Parser();
  ~Parser();
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
  std::size_t position = 0;
  int line = 1;
  int column = 1;
};

} // namespace Amanuensis
