#include "writer-context.hpp"
#include "amanuensis/core/value.hpp"
#include "core/number-format.hpp"
#include "core/quoted-string/quoted-string.hpp"
#include <cmath>

namespace amanuensis::json {

void WriterContext::WriteIndent(std::string& output, int depth, const WriterOptions& options)
{
  if (!options.pretty) {
    return;
  }
  int totalSpaces = depth * options.indentWidth;
  output.append(static_cast<std::size_t>(totalSpaces), options.indentChar);
}

void WriterContext::WriteArray(
    std::string& output,
    const core::Value& arrayValue,
    int depth,
    const WriterOptions& options
)
{
  const auto& elements = arrayValue.AsArray();
  if (elements.empty()) {
    output.append("[]");
    return;
  }

  output.push_back('[');
  if (options.pretty) {
    output.push_back('\n');
  }

  for (std::size_t elementIndex = 0; elementIndex < elements.size(); ++elementIndex) {
    WriteIndent(output, depth + 1, options);
    WriteValue(output, elements[elementIndex], depth + 1, options);
    if (elementIndex + 1 < elements.size()) {
      output.push_back(',');
    }
    if (options.pretty) {
      output.push_back('\n');
    }
  }

  this->WriteIndent(output, depth, options);
  output.push_back(']');
}

void WriterContext::WriteObject(
    std::string& output,
    const core::Value& objectValue,
    int depth,
    const WriterOptions& options
)
{
  if (objectValue.Size() == 0) {
    output.append("{}");
    return;
  }

  output.push_back('{');
  if (options.pretty) {
    output.push_back('\n');
  }

  std::size_t entryIndex = 0;
  std::size_t totalEntries = objectValue.Size();

  for (auto iterator = objectValue.BeginObject(); iterator != objectValue.EndObject(); ++iterator) {
    WriteIndent(output, depth + 1, options);
    core::AppendQuotedString(output, iterator->first);
    output.push_back(':');
    if (options.pretty) {
      output.push_back(' ');
    }
    WriteValue(output, iterator->second, depth + 1, options);
    if (entryIndex + 1 < totalEntries) {
      output.push_back(',');
    }
    if (options.pretty) {
      output.push_back('\n');
    }
    ++entryIndex;
  }

  this->WriteIndent(output, depth, options);
  output.push_back('}');
}

void WriterContext::WriteValue(
    std::string& output,
    const core::Value& value,
    int depth,
    const WriterOptions& options
)
{
  switch (value.GetType()) {
  case core::ValueType::Null:
    output.append("null");
    break;

  case core::ValueType::Boolean:
    output.append(value.AsBoolean() ? "true" : "false");
    break;

  case core::ValueType::Integer:
    core::AppendInteger(output, value.AsInteger());
    break;

  case core::ValueType::Double: {
    double doubleValue = value.AsDouble();
    if (std::isnan(doubleValue) || std::isinf(doubleValue)) {
      // JSON has no NaN/Inf — emit null as a safe fallback.
      output.append("null");
    }
    else {
      core::AppendDouble(output, doubleValue);
    }
    break;
  }

  case core::ValueType::String:
    core::AppendQuotedString(output, value.AsString());
    break;

  case core::ValueType::Array:
    WriteArray(output, value, depth, options);
    break;

  case core::ValueType::Object:
    WriteObject(output, value, depth, options);
    break;
  }
}
} // namespace amanuensis::json
