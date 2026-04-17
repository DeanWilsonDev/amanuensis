#pragma once

#include "value.hpp"
#include <filesystem>

namespace Amanuensis {

struct WriterOptions {
  bool pretty = true;
  int indentWidth = 2;
  char indentChar = ' ';
  bool trailingNewline = true;
};

class Writer {
public:
  static std::string WriteToString(const Value& value, const WriterOptions& options = {});
  static bool WriteToFile(
      const Value& value,
      const std::filesystem::path& path,
      const WriterOptions& options = {}
  );
};

} // namespace Amanuensis
