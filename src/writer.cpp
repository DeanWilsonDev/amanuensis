#include "amanuensis/writer.hpp"

#include <charconv>
#include <cmath>
#include <fstream>
#include <sstream>

namespace Amanuensis {

// -----------------------------------------------------------------------
// Internal recursive serialiser.  Walks the Value tree and appends
// formatted JSON text to an output string.
// -----------------------------------------------------------------------

namespace {

void WriteIndent(std::string& output, int depth, const WriterOptions& options) {
  if (!options.pretty) {
    return;
  }
  int totalSpaces = depth * options.indentWidth;
  output.append(static_cast<std::size_t>(totalSpaces), options.indentChar);
}

void WriteEscapedString(std::string& output, const std::string& text) {
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
        std::snprintf(hexBuffer, sizeof(hexBuffer), "\\u%04x",
                      static_cast<unsigned char>(character));
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

void WriteValue(std::string& output, const Value& value, int depth, const WriterOptions& options);

void WriteArray(
    std::string& output,
    const Value& arrayValue,
    int depth,
    const WriterOptions& options
) {
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

  WriteIndent(output, depth, options);
  output.push_back(']');
}

void WriteObject(
    std::string& output,
    const Value& objectValue,
    int depth,
    const WriterOptions& options
) {
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

  for (auto iterator = objectValue.BeginObject(); iterator != objectValue.EndObject();
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

  WriteIndent(output, depth, options);
  output.push_back('}');
}

void WriteValue(
    std::string& output,
    const Value& value,
    int depth,
    const WriterOptions& options
) {
  switch (value.GetType()) {
  case ValueType::Null:
    output.append("null");
    break;

  case ValueType::Boolean:
    output.append(value.AsBoolean() ? "true" : "false");
    break;

  case ValueType::Integer: {
    // std::to_string is fine for integers
    output.append(std::to_string(value.AsInteger()));
    break;
  }

  case ValueType::Double: {
    // Use enough precision for lossless round-trip.
    // 17 significant digits is sufficient for IEEE 754 double.
    double doubleValue = value.AsDouble();
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
    WriteEscapedString(output, value.AsString());
    break;

  case ValueType::Array:
    WriteArray(output, value, depth, options);
    break;

  case ValueType::Object:
    WriteObject(output, value, depth, options);
    break;
  }
}

} // anonymous namespace

// -----------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------

std::string Writer::WriteToString(const Value& value, const WriterOptions& options) {
  std::string output;
  WriteValue(output, value, 0, options);
  if (options.trailingNewline) {
    output.push_back('\n');
  }
  return output;
}

bool Writer::WriteToFile(
    const Value& value,
    const std::filesystem::path& path,
    const WriterOptions& options
) {
  std::string jsonText = WriteToString(value, options);
  std::ofstream outputFile(path, std::ios::binary);
  if (!outputFile.is_open()) {
    return false;
  }
  outputFile.write(jsonText.data(), static_cast<std::streamsize>(jsonText.size()));
  return outputFile.good();
}

} // namespace Amanuensis
