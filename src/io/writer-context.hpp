#pragma once

#include <amanuensis/io/writer-options.hpp>
#include <amanuensis/json-value.hpp>
#include <string>

namespace Amanuensis {

class WriterContext {
public:
  void WriteIndent(std::string& output, int depth, const WriterOptions& options);

  void WriteEscapedString(std::string& output, const std::string& text);

  void
  WriteArray(std::string& output, const JsonValue& arrayJsonValue, int depth, const WriterOptions& options);

  void WriteObject(
      std::string& output,
      const JsonValue& objectJsonValue,
      int depth,
      const WriterOptions& options
  );

  void WriteJsonValue(std::string& output, const JsonValue& value, int depth, const WriterOptions& options);
};
} // namespace Amanuensis
