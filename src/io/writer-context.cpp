#include "writer-context.hpp"
#include "amanuensis/json.hpp"
#include <charconv>
#include <cmath>

namespace Amanuensis {

void WriterContext::WriteIndent(std::string& output, int depth, const WriterOptions& options)
{
  if (!options.pretty) {
    return;
  }
  int totalSpaces = depth * options.indentWidth;
  output.append(static_cast<std::size_t>(totalSpaces), options.indentChar);
}

void WriterContext::WriteEscapedString(std::string& output, const std::string& text)
{
  output.push_back('"');
  for (char character : text) {
    switch (character) {
    case '"':
      output.append("\\\"");
      break;
    case '\\':
      output.append("\\\\");
      break;
    case '\b':
      output.append("\\b");
      break;
    case '\f':
      output.append("\\f");
      break;
    case '\n':
      output.append("\\n");
      break;
    case '\r':
      output.append("\\r");
      break;
    case '\t':
      output.append("\\t");
      break;
    default:
      if (static_cast<unsigned char>(character) < 0x20) {
        // Control characters — emit as \u00XX
        char hexBuffer[8];
        std::snprintf(
            hexBuffer, sizeof(hexBuffer), "\\u%04x", static_cast<unsigned char>(character)
        );
        output.append(hexBuffer);
      }
      else {
        output.push_back(character);
      }
      break;
    }
  }
  output.push_back('"');
}

void WriterContext::WriteArray(
    std::string& output,
    const Value& arrayValue,
    int depth,
    const WriterOptions& options
)
{
  const auto& elements = Json::AsArray(arrayValue);
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
    const Value& objectValue,
    int depth,
    const WriterOptions& options
)
{
  if (Json::Size(objectValue) == 0) {
    output.append("{}");
    return;
  }

  output.push_back('{');
  if (options.pretty) {
    output.push_back('\n');
  }

  std::size_t entryIndex = 0;
  std::size_t totalEntries = Json::Size(objectValue);

  for (auto iterator = Json::BeginObject(objectValue); iterator != Json::EndObject(objectValue);
       ++iterator) {
    WriteIndent(output, depth + 1, options);
    WriteEscapedString(output, iterator->first);
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
    const Value& value,
    int depth,
    const WriterOptions& options
)
{
  switch (Json::GetType(value)) {
  case ValueType::Null:
    output.append("null");
    break;

  case ValueType::Boolean:
    output.append(Json::AsBoolean(value) ? "true" : "false");
    break;

  case ValueType::Integer: {
    // std::to_string is fine for integers
    output.append(std::to_string(Json::AsInteger(value)));
    break;
  }

  case ValueType::Double: {
    // Use enough precision for lossless round-trip.
    // 17 significant digits is sufficient for IEEE 754 double.
    double doubleValue = Json::AsDouble(value);
    if (std::isnan(doubleValue) || std::isinf(doubleValue)) {
      // JSON has no NaN/Inf — emit null as a safe fallback.
      output.append("null");
    }
    else {
      char formatBuffer[64];
      auto [endPointer, errorCode] =
          std::to_chars(formatBuffer, formatBuffer + sizeof(formatBuffer), doubleValue);
      std::string_view formatted(formatBuffer, static_cast<std::size_t>(endPointer - formatBuffer));

      // Ensure there's a decimal point so the output parses back as a double,
      // not as an integer.
      bool hasDecimalPoint = false;
      for (char c : formatted) {
        if (c == '.' || c == 'e' || c == 'E') {
          hasDecimalPoint = true;
          break;
        }
      }
      output.append(formatted);
      if (!hasDecimalPoint) {
        output.append(".0");
      }
    }
    break;
  }

  case ValueType::String:
    WriteEscapedString(output, Json::AsString(value));
    break;

  case ValueType::Array:
    WriteArray(output, value, depth, options);
    break;

  case ValueType::Object:
    WriteObject(output, value, depth, options);
    break;
  }
}
} // namespace Amanuensis
