#include "calamus/writer/value-writer.hpp"
#include "calamus/canonical.hpp"
#include "core/number-format.hpp"
#include "core/quoted-string/quoted-string.hpp"

#include <cmath>
#include <variant>
#include <vector>

namespace amanuensis::calamus {

using Array = std::vector<core::Value>;
using Object = core::OrderedMap<core::Value>;

bool IsInline(const core::Value& value)
{
  if (std::holds_alternative<Object>(value.data)) {
    return false;
  }
  if (const auto* array = std::get_if<Array>(&value.data)) {
    for (const core::Value& element : *array) {
      if (!IsInline(element)) {
        return false;
      }
    }
  }
  return true;
}

static bool AppendInlineArray(std::string& output, const Array& array)
{
  output.push_back('[');
  bool first = true;
  for (const core::Value& element : array) {
    if (!first) {
      output.append(", ");
    }
    first = false;
    if (!AppendInlineValue(output, element)) {
      return false;
    }
  }
  output.push_back(']');
  return true;
}

bool AppendInlineValue(std::string& output, const core::Value& value)
{
  if (std::holds_alternative<std::monostate>(value.data)) {
    output.append("null");
  }
  else if (const auto* boolean = std::get_if<bool>(&value.data)) {
    output.append(*boolean ? "true" : "false");
  }
  else if (const auto* integer = std::get_if<long long>(&value.data)) {
    core::AppendInteger(output, *integer);
  }
  else if (const auto* number = std::get_if<double>(&value.data)) {
    if (!std::isfinite(*number)) {
      return false;
    }
    AppendCanonicalDouble(output, *number);
  }
  else if (const auto* text = std::get_if<std::string>(&value.data)) {
    core::AppendQuotedString(output, *text);
  }
  else {
    return AppendInlineArray(output, std::get<Array>(value.data));
  }
  return true;
}

} // namespace amanuensis::calamus
