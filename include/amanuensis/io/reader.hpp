#pragma once

#include <filesystem>
#include "parse-result.hpp"

namespace Amanuensis {

class Reader {
public:
  Reader();
  static ParseResult ParseString(std::string_view text);
  static ParseResult ParseFile(const std::filesystem::path& path);
};

} // namespace Amanuensis
