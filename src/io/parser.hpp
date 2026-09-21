#pragma once

#include <amanuensis/io/json-parse-result.hpp>
#include "amanuensis/json-value.hpp"
#include <string>
#include <cstdint>

namespace Amanuensis {

class Parser {
public:
  Parser(std::string_view input);
  ~Parser();

  JsonParseResult Parse();
  bool IsAtEnd() const;
  char Peek() const;
  char Advance();
  JsonParseResult MakeError(const std::string& message) const;

  void SkipWhitespace();
  JsonParseResult ParseNull();
  JsonParseResult ParseString();
  JsonParseResult ParseNumber();
  JsonParseResult ParseArray();
  JsonParseResult ParseObject();
  JsonParseResult ParseTrue();
  JsonParseResult ParseFalse();
  JsonParseResult ParseJsonValue();
  void SetInput(std::string_view input);

private:
  static int HexDigitJsonValue(char character);
  bool ParseFourHexDigits(uint16_t& outCodeUnit);

  std::string_view input;
  std::size_t cursor = 0;
  int line = 1;
  int column = 1;
};

} // namespace Amanuensis
