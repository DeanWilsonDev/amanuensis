#pragma once

#include <amanuensis/core/value.hpp>
#include <amanuensis/json/writer-options.hpp>
#include <filesystem>

namespace amanuensis::json {

class Writer {
public:
  static std::string WriteToString(const core::Value& value, const WriterOptions& options = {});
  static bool WriteToFile(
      const core::Value& value,
      const std::filesystem::path& path,
      const WriterOptions& options = {}
  );
};

} // namespace amanuensis::json
