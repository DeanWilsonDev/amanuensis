#include "calamus/canonical.hpp"

#include "core/number-format.hpp"

#include <charconv>
#include <cmath>
#include <limits>
#include <utility>
#include <variant>
#include <vector>

namespace amanuensis::calamus {

// Converting a double outside float's range to float is undefined, so the
// range is checked first. NaN fails every comparison.
static bool IsExactlyAFloat(double value)
{
  return std::fabs(value) <= std::numeric_limits<float>::max() &&
         static_cast<double>(static_cast<float>(value)) == value;
}

void AppendCanonicalDouble(std::string& output, double value)
{
  if (IsExactlyAFloat(value)) {
    core::AppendFloat(output, static_cast<float>(value));
  }
  else {
    core::AppendDouble(output, value);
  }
}

double CanonicalDouble(double value)
{
  if (!std::isfinite(value)) {
    return value;
  }
  std::string text;
  AppendCanonicalDouble(text, value);
  double canonical = 0.0;
  std::from_chars(text.data(), text.data() + text.size(), canonical);
  return canonical;
}

core::Value Canonicalise(const core::Value& value)
{
  if (const auto* number = std::get_if<double>(&value.data)) {
    return core::Value{CanonicalDouble(*number)};
  }
  if (const auto* array = std::get_if<std::vector<core::Value>>(&value.data)) {
    std::vector<core::Value> canonical;
    canonical.reserve(array->size());
    for (const core::Value& element : *array) {
      canonical.push_back(Canonicalise(element));
    }
    return core::Value{std::move(canonical)};
  }
  if (const auto* object = std::get_if<core::OrderedMap<core::Value>>(&value.data)) {
    core::OrderedMap<core::Value> canonical;
    for (const auto& [key, entry] : object->GetEntries()) {
      canonical.Insert(key, Canonicalise(entry));
    }
    return core::Value{std::move(canonical)};
  }
  return value;
}

} // namespace amanuensis::calamus
