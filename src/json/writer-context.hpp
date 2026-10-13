#pragma once

#include <amanuensis/json/writer-options.hpp>
#include <amanuensis/core/value.hpp>
#include <string>

namespace amanuensis::json {

class WriterContext {
public:
  void WriteIndent(std::string& output, int depth, const WriterOptions& options);

  void WriteArray(
      std::string& output,
      const core::Value& arrayValue,
      int depth,
      const WriterOptions& options
  );

  void WriteObject(
      std::string& output,
      const core::Value& objectValue,
      int depth,
      const WriterOptions& options
  );

  void WriteValue(
      std::string& output,
      const core::Value& value,
      int depth,
      const WriterOptions& options
  );
};
} // namespace amanuensis::json
