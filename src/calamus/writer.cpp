#include "amanuensis/calamus/writer.hpp"
#include "amanuensis/core/diff.hpp"
#include "amanuensis/core/key-path.hpp"
#include "amanuensis/core/stable-hash.hpp"
#include "calamus/canonical.hpp"
#include "calamus/syntax.hpp"
#include "core/number-format.hpp"
#include "core/quoted-string.hpp"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <set>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace amanuensis::calamus {

namespace {

using Array = std::vector<core::Value>;
using Object = core::OrderedMap<core::Value>;

// Where the entity differs from its base, from core::Diff over the canonical
// values.
struct Changes {
  // The paths Diff reported. Every line at or under one is marked.
  std::set<core::KeyPath> whole;
  // The paths with a reported path somewhere under them. Their block lines
  // are marked.
  std::set<core::KeyPath> containing;
};

// The value at path, or nullptr when some key on the way is missing or isn't
// an Object.
const core::Value* FindPath(const core::Value& entity, const core::KeyPath& path)
{
  const core::Value* current = &entity;
  for (const std::string& key : path) {
    const auto* object = std::get_if<Object>(&current->data);
    if (object == nullptr || (current = object->Find(key)) == nullptr) {
      return nullptr;
    }
  }
  return current;
}

bool EntityHas(const core::Value& entity, const core::KeyPath& path)
{
  return FindPath(entity, path) != nullptr;
}

// The Object at path, or nullptr when there's none.
const core::Value* FindObject(const core::Value& entity, const core::KeyPath& path)
{
  const core::Value* found = FindPath(entity, path);
  return found != nullptr && std::holds_alternative<Object>(found->data) ? found : nullptr;
}

// Whether path starts with prefix, or is prefix.
bool IsUnder(const core::KeyPath& path, const core::KeyPath& prefix)
{
  return path.size() >= prefix.size() && std::equal(prefix.begin(), prefix.end(), path.begin());
}

Changes FindChanges(
    const core::Value& canonicalBase,
    const core::Value& canonicalEntity,
    const std::vector<core::KeyPath>& atomic
)
{
  Changes changes;
  for (core::KeyPath& path : core::Diff(canonicalBase, canonicalEntity)) {
    // A key only the base has can't be written, so nothing marks it.
    if (!EntityHas(canonicalEntity, path)) {
      continue;
    }
    for (std::size_t length = 0; length < path.size(); ++length) {
      changes.containing.insert(core::KeyPath(path.begin(), path.begin() + length));
    }
    changes.whole.insert(std::move(path));
  }

  // An atomic object with a change inside it changes as a whole. Paths with
  // the same prefix sort together, so the first one at or after the atomic
  // path is under it if any is.
  for (const core::KeyPath& atomicPath : atomic) {
    const core::Value* object = FindObject(canonicalEntity, atomicPath);
    auto firstUnder = changes.whole.lower_bound(atomicPath);
    if (object != nullptr && firstUnder != changes.whole.end() &&
        IsUnder(*firstUnder, atomicPath)) {
      changes.whole.insert(atomicPath);
    }
  }
  return changes;
}

// Scalars, and arrays that hold no Object at any depth, fit on one line.
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

void AppendKey(std::string& output, const std::string& key)
{
  if (IsBareKey(key)) {
    output.append(key);
  }
  else {
    core::AppendQuotedString(output, key);
  }
}

std::string FormatPath(const core::KeyPath& path)
{
  std::string formatted;
  for (const std::string& key : path) {
    if (!formatted.empty()) {
      formatted.push_back('.');
    }
    AppendKey(formatted, key);
  }
  return formatted;
}

void AppendHex(std::string& output, std::uint64_t value)
{
  char digits[16];
  auto [endPointer, errorCode] = std::to_chars(digits, digits + sizeof(digits), value, 16);
  output.append(sizeof(digits) - static_cast<std::size_t>(endPointer - digits), '0');
  output.append(digits, endPointer);
}

// Writes the body one line at a time. Marks come from Changes outside arrays;
// inside an array every line takes the array's mark.
class BodyWriter {
public:
  BodyWriter(std::string& output, const Changes& changes)
      : output_(output)
      , changes_(changes)
  {
  }

  const std::string& Error() const { return error_; }

  // base is the base value at this object's path, or nullptr when the base has
  // none or the object is inside an array. markAll marks every line.
  bool WriteObject(
      const Object& object,
      const core::Value* base,
      bool insideArray,
      bool markAll,
      int depth
  )
  {
    const auto* baseObject = base != nullptr ? std::get_if<Object>(&base->data) : nullptr;
    for (const auto& [key, value] : object.GetEntries()) {
      path_.push_back(key);
      bool whole = markAll || (!insideArray && changes_.whole.contains(path_));
      bool marked = whole || (!insideArray && changes_.containing.contains(path_));
      const core::Value* baseValue = baseObject != nullptr ? baseObject->Find(key) : nullptr;

      StartLine(marked, depth);
      AppendKey(output_, key);
      if (const auto* child = std::get_if<Object>(&value.data)) {
        output_.append(":\n");
        if (!WriteObject(*child, baseValue, insideArray, whole, depth + 1)) {
          return false;
        }
      }
      else if (!IsInline(value)) {
        output_.append(":\n");
        if (!WriteItems(std::get<Array>(value.data), marked, depth + 1)) {
          return false;
        }
      }
      else {
        output_.append(" : ");
        if (!AppendValue(value, "")) {
          return false;
        }
        if (marked && baseValue != nullptr && IsInline(*baseValue)) {
          output_.append(" | ");
          if (!AppendValue(*baseValue, " in the base")) {
            return false;
          }
        }
        output_.push_back('\n');
      }
      path_.pop_back();
    }
    return true;
  }

private:
  bool WriteItems(const Array& array, bool marked, int depth)
  {
    for (const core::Value& element : array) {
      StartLine(marked, depth);
      if (const auto* object = std::get_if<Object>(&element.data)) {
        output_.append("-\n");
        if (!WriteObject(*object, nullptr, true, marked, depth + 1)) {
          return false;
        }
      }
      else if (!IsInline(element)) {
        output_.append("-\n");
        if (!WriteItems(std::get<Array>(element.data), marked, depth + 1)) {
          return false;
        }
      }
      else {
        output_.append("- ");
        if (!AppendValue(element, "")) {
          return false;
        }
        output_.push_back('\n');
      }
    }
    return true;
  }

  void StartLine(bool marked, int depth)
  {
    output_.push_back(marked ? '*' : ' ');
    output_.push_back(' ');
    output_.append(static_cast<std::size_t>(depth) * 2, ' ');
  }

  // A scalar or inline array. where says whose value it is, for the error.
  bool AppendValue(const core::Value& value, const char* where)
  {
    if (std::holds_alternative<std::monostate>(value.data)) {
      output_.append("null");
    }
    else if (const auto* boolean = std::get_if<bool>(&value.data)) {
      output_.append(*boolean ? "true" : "false");
    }
    else if (const auto* integer = std::get_if<long long>(&value.data)) {
      core::AppendInteger(output_, *integer);
    }
    else if (const auto* number = std::get_if<double>(&value.data)) {
      if (!std::isfinite(*number)) {
        error_ = "Can't write NaN or infinity" + std::string(where) + " at " + FormatPath(path_) +
                 "; Calamus has no syntax for them";
        return false;
      }
      AppendCanonicalDouble(output_, *number);
    }
    else if (const auto* text = std::get_if<std::string>(&value.data)) {
      core::AppendQuotedString(output_, *text);
    }
    else {
      output_.push_back('[');
      bool first = true;
      for (const core::Value& element : std::get<Array>(value.data)) {
        if (!first) {
          output_.append(", ");
        }
        first = false;
        if (!AppendValue(element, where)) {
          return false;
        }
      }
      output_.push_back(']');
    }
    return true;
  }

  std::string& output_;
  const Changes& changes_;
  core::KeyPath path_;
  std::string error_;
};

WriteResult Fail(std::string error)
{
  return WriteResult{false, std::string(), std::move(error)};
}

} // namespace

WriteResult Writer::WriteToString(
    const Header& header,
    const core::Value& entity,
    const core::Value& base,
    const WriterOptions& options
)
{
  const auto* entityObject = std::get_if<Object>(&entity.data);
  if (entityObject == nullptr) {
    return Fail("The entity must be an Object");
  }
  if (header.version < 1) {
    return Fail("@version must be at least 1");
  }
  if (!IsTypeName(header.type)) {
    return Fail("@type must be a name: a letter or '_', then letters, digits, '_', '.' or ':'");
  }

  core::Value canonicalBase = Canonicalise(base);
  Changes changes = FindChanges(canonicalBase, Canonicalise(entity), options.atomic);

  std::string body;
  BodyWriter bodyWriter(body, changes);
  if (!bodyWriter.WriteObject(*entityObject, &base, false, changes.whole.contains({}), 0)) {
    return Fail(bodyWriter.Error());
  }

  std::string text = "@version " + std::to_string(header.version) + "\n@type " + header.type;
  if (header.id) {
    text.append("\n@id ");
    AppendHex(text, *header.id);
  }
  if (header.base) {
    text.append("\n@base ");
    core::AppendQuotedString(text, *header.base);
  }
  text.append("\n@base_hash ");
  AppendHex(text, core::StableHash(canonicalBase));
  text.push_back('\n');
  if (!body.empty()) {
    text.push_back('\n');
    text.append(body);
  }
  return WriteResult{true, std::move(text), std::string()};
}

WriteResult Writer::WriteToFile(
    const Header& header,
    const core::Value& entity,
    const core::Value& base,
    const std::filesystem::path& path,
    const WriterOptions& options
)
{
  WriteResult result = WriteToString(header, entity, base, options);
  if (!result.succeeded) {
    return result;
  }
  std::ofstream outputFile(path, std::ios::binary);
  if (!outputFile.is_open()) {
    return Fail("Could not open file: " + path.string());
  }
  outputFile.write(result.text.data(), static_cast<std::streamsize>(result.text.size()));
  if (!outputFile.good()) {
    return Fail("Could not write file: " + path.string());
  }
  return result;
}

} // namespace amanuensis::calamus
