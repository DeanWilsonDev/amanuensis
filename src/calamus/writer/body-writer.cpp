#include "calamus/writer/body-writer.hpp"
#include "calamus/writer/key-writer.hpp"
#include "calamus/writer/value-writer.hpp"

#include <cstddef>
#include <variant>

namespace amanuensis::calamus {

using Array = std::vector<core::Value>;
using Object = core::OrderedMap<core::Value>;

bool BodyWriter::WriteObject(
    const Object& object,
    const core::Value* base,
    bool insideArray,
    bool markAll,
    int depth
)
{
  const auto* baseObject = base != nullptr ? std::get_if<Object>(&base->data) : nullptr;
  for (const auto& [key, value] : object.GetEntries()) {
    const core::Value* baseValue = baseObject != nullptr ? baseObject->Find(key) : nullptr;
    this->path.push_back(key);
    if (!this->WriteEntry(key, value, baseValue, insideArray, markAll, depth)) {
      return false;
    }
    this->path.pop_back();
  }
  return true;
}

bool BodyWriter::WriteEntry(
    const std::string& key,
    const core::Value& value,
    const core::Value* baseValue,
    bool insideArray,
    bool markAll,
    int depth
)
{
  bool whole = markAll || (!insideArray && this->changes.whole.contains(this->path));
  bool marked = whole || (!insideArray && this->changes.containing.contains(this->path));

  this->StartLine(marked, depth);
  AppendKey(this->output, key);
  if (const auto* child = std::get_if<Object>(&value.data)) {
    this->output.append(":\n");
    return this->WriteObject(*child, baseValue, insideArray, whole, depth + 1);
  }
  if (!IsInline(value)) {
    this->output.append(":\n");
    return this->WriteItems(std::get<Array>(value.data), marked, depth + 1);
  }

  this->output.append(" : ");
  if (!this->AppendValue(value, "")) {
    return false;
  }
  if (marked && baseValue != nullptr && IsInline(*baseValue)) {
    this->output.append(" | ");
    if (!this->AppendValue(*baseValue, " in the base")) {
      return false;
    }
  }
  this->output.push_back('\n');
  return true;
}

bool BodyWriter::WriteItems(const Array& array, bool marked, int depth)
{
  for (const core::Value& element : array) {
    this->StartLine(marked, depth);
    if (const auto* object = std::get_if<Object>(&element.data)) {
      this->output.append("-\n");
      if (!this->WriteObject(*object, nullptr, true, marked, depth + 1)) {
        return false;
      }
    }
    else if (!IsInline(element)) {
      this->output.append("-\n");
      if (!this->WriteItems(std::get<Array>(element.data), marked, depth + 1)) {
        return false;
      }
    }
    else {
      this->output.append("- ");
      if (!this->AppendValue(element, "")) {
        return false;
      }
      this->output.push_back('\n');
    }
  }
  return true;
}

void BodyWriter::StartLine(bool marked, int depth)
{
  this->output.push_back(marked ? '*' : ' ');
  this->output.push_back(' ');
  this->output.append(static_cast<std::size_t>(depth) * 2, ' ');
}

bool BodyWriter::AppendValue(const core::Value& value, const char* where)
{
  if (AppendInlineValue(this->output, value)) {
    return true;
  }
  this->error = "Can't write NaN or infinity" + std::string(where) + " at " +
                FormatPath(this->path) + "; Calamus has no syntax for them";
  return false;
}

} // namespace amanuensis::calamus
