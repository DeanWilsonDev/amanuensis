#pragma once

#include "amanuensis/parser.hpp"
#include <filesystem>

namespace Amanuensis {

struct ParseResult;

class Reader {
public:
  Reader();
  ParseResult ParseString(std::string_view text);
  ParseResult ParseFile(const std::filesystem::path& path);

private:
  Parser parser;
};

} // namespace Amanuensis
