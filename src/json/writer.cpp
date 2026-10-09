#include <amanuensis/json/writer.hpp>
#include "writer-context.hpp"

#include <fstream>

namespace amanuensis::json {

std::string Writer::WriteToString(const core::Value& value, const WriterOptions& options)
{
  std::string output;
  WriterContext().WriteValue(output, value, 0, options);
  if (options.trailingNewline) {
    output.push_back('\n');
  }
  return output;
}

bool Writer::WriteToFile(
    const core::Value& value,
    const std::filesystem::path& path,
    const WriterOptions& options
)
{
  std::string jsonText = WriteToString(value, options);
  std::ofstream outputFile(path, std::ios::binary);
  if (!outputFile.is_open()) {
    return false;
  }
  outputFile.write(jsonText.data(), static_cast<std::streamsize>(jsonText.size()));
  return outputFile.good();
}

} // namespace amanuensis::json
