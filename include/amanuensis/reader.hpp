#pragma once

#include "value.hpp"
#include <filesystem>
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

class Reader {
public:
  static ParseResult ParseString(std::string_view text);
  static ParseResult ParseFile(const std::filesystem::path& path);
};

} // namespace Amanuensis
