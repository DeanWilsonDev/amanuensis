#pragma once

#include "amanuensis/calamus/header.hpp"
#include "amanuensis/core/key-path.hpp"
#include "amanuensis/core/parse-error.hpp"
#include "amanuensis/core/value.hpp"

#include <filesystem>
#include <optional>
#include <string_view>
#include <vector>

namespace amanuensis::calamus {

// A body line whose mark the reader honours, because it holds a whole field
// value: a field line, or a block line whose block is an array or an empty
// object. Lines inside an array block are never fields, since an array
// overrides as a whole.
struct Field {
  core::KeyPath path;
  core::Value value;
  bool marked = false;
  // The base value recorded after '|'. It's never loaded, only compared with
  // the current base for the override review.
  std::optional<core::Value> recordedDefault;
  int line = 0;
};

struct Document {
  Header header;
  // Every body line's value with the marks ignored. The root is an Object.
  core::Value body;
  std::vector<Field> fields;
};

struct ReadResult {
  bool succeeded;
  Document document;
  core::ParseError error;
};

// Each list holds key paths in document order.
struct LoadResult {
  core::Value value;

  std::vector<core::KeyPath> overrides;

  // Set only by a person's edit (promoted or reset). Changes to the base never
  // set it; they wait for the next save.
  bool needsRewrite = false;

  // Whether @base_hash matches the base, or is absent.
  bool baseHashMatches = true;

  // Unmarked fields that differ from an unchanged base: hand edits that forgot
  // their '*', kept as overrides.
  std::vector<core::KeyPath> promoted;

  // Marked fields that equal an unchanged base, so they follow it.
  std::vector<core::KeyPath> reset;

  // Unmarked fields that differ from a base that changed since the file was
  // written. They follow the new base.
  std::vector<core::KeyPath> refreshed;

  // Unmarked fields that a changed base no longer has. They're dropped.
  std::vector<core::KeyPath> stale;

  // Marked fields the base doesn't have, kept as overrides.
  std::vector<core::KeyPath> additions;

  // Overrides whose recorded default no longer matches the base: the base
  // moved under them, so a designer may want to check them.
  std::vector<core::KeyPath> moved;
};

class Reader {
public:
  // Reads the syntax only, so the caller can find the base the header names
  // before calling Load.
  static ReadResult ParseString(std::string_view text);
  static ReadResult ParseFile(const std::filesystem::path& path);

  // base is the prefab, or the type's default when there's no @base. Values are
  // compared and hashed canonically, so a float field matches its base after a
  // round trip through text.
  static LoadResult Load(const Document& document, const core::Value& base);
};

} // namespace amanuensis::calamus
