#pragma once

#include <amanuensis/value.hpp>
#include <amanuensis/io/writer-options.hpp>
#include <filesystem>

namespace amanuensis {

class Writer {
public:
  static std::string WriteToString(const Value& value, const WriterOptions& options = {});
  static bool WriteToFile(
      const Value& value,
      const std::filesystem::path& path,
      const WriterOptions& options = {}
  );
};

} // namespace amanuensis
