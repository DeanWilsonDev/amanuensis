#pragma once

#include <amanuensis/io/writer-options.hpp>
#include <amanuensis/value.hpp>
#include <string>

namespace Amanuensis {

class WriterContext {
public:
  void WriteIndent(std::string& output, int depth, const WriterOptions& options);

  void WriteEscapedString(std::string& output, const std::string& text);

  void
  WriteArray(std::string& output, const Value& arrayValue, int depth, const WriterOptions& options);

  void WriteObject(
      std::string& output,
      const Value& objectValue,
      int depth,
      const WriterOptions& options
  );

  void WriteValue(std::string& output, const Value& value, int depth, const WriterOptions& options);
};
} // namespace Amanuensis
