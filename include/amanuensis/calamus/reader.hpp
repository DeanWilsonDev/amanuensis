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

class Reader {
public:
  // Read the syntax only, so the caller can find the base the header names.
  // Every error stops the read and reports a line and column. Never throws on
  // bad input.
  static ReadResult ParseString(std::string_view text);
  static ReadResult ParseFile(const std::filesystem::path& path);
};

} // namespace amanuensis::calamus
