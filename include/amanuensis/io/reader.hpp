#pragma once

#include <filesystem>
#include "amanuensis/io/json-parse-result.hpp"

namespace Amanuensis {

class Reader {
public:
  Reader();
  static JsonParseResult ParseString(std::string_view text);
  static JsonParseResult ParseFile(const std::filesystem::path& path);
};

} // namespace Amanuensis
