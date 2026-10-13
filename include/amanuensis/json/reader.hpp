#pragma once

#include <filesystem>
#include "amanuensis/core/parse-result.hpp"

namespace amanuensis::json {

class Reader {
public:
  Reader();
  static core::ParseResult ParseString(std::string_view text);
  static core::ParseResult ParseFile(const std::filesystem::path& path);
};

} // namespace amanuensis::json
