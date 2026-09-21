#pragma once

#include <amanuensis/json-value.hpp>
#include <amanuensis/io/writer-options.hpp>
#include <filesystem>

namespace Amanuensis {

class Writer {
public:
  static std::string WriteToString(const JsonValue& value, const WriterOptions& options = {});
  static bool WriteToFile(
      const JsonValue& value,
      const std::filesystem::path& path,
      const WriterOptions& options = {}
  );
};

} // namespace Amanuensis
