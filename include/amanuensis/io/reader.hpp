#pragma once

#include <filesystem>
#include "amanuensis/io/parse-result.hpp"

namespace amanuensis {

class Reader {
public:
  Reader();
  static ParseResult ParseString(std::string_view text);
  static ParseResult ParseFile(const std::filesystem::path& path);
};

} // namespace amanuensis
