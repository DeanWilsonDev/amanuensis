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

// A Calamus file as written, before it is applied over its base.
struct Document {
  Header header;
  // Every body line's value with the marks ignored. The root is an Object.
  core::Value body;
  // The fields, in document order.
  std::vector<Field> fields;
};

struct ReadResult {
  bool succeeded;
  Document document;
  core::ParseError error;
};

// What Load made of a document. Each list holds key paths in document order.
// See the decision table in docs/calamus-spec.md, "Load".
struct LoadResult {
  // The entity: the base with the overrides applied.
  core::Value value;

  // The fields that override the base. They're the ones the writer will mark.
  std::vector<core::KeyPath> overrides;

  // A person's edit to this file (promoted or reset) means regenerating it may
  // change its text. The caller regenerates it and writes it only if the text
  // differs. Changes to the base never set this; they wait for the next save.
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
  // Read the syntax only, so the caller can find the base the header names.
  // Every error stops the read and reports a line and column. Never throws on
  // bad input.
  static ReadResult ParseString(std::string_view text);
  static ReadResult ParseFile(const std::filesystem::path& path);

  // Applies a document's fields over its base: the prefab, or the type's
  // default when there's no @base. Marked fields override the base and
  // unmarked ones follow it, healing hand edits through @base_hash. Values are
  // compared and hashed canonically, so a float field matches its base after
  // a round trip through text. Never writes anything.
  static LoadResult Load(const Document& document, const core::Value& base);
};

} // namespace amanuensis::calamus
