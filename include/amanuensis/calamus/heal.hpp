#pragma once

#include "amanuensis/calamus/reader.hpp"
#include "amanuensis/core/parse-error.hpp"
#include "amanuensis/core/value.hpp"

#include <filesystem>

namespace amanuensis::calamus {

struct HealResult {
  bool succeeded;
  // The file's text changed, so it was written back.
  bool rewritten;
  // What loading the file found, including why it needed healing.
  LoadResult load;
  // A parse error has a line and column. A file or write error has line 0.
  core::ParseError error;
};

// Loads the file at path over base. When that finds a person's edit to the
// file (LoadResult::needsRewrite), regenerates the text and writes it back,
// but only if it differs from the text on disk, so a clean file never
// produces a diff. A change to the base alone never rewrites the file; it
// waits for the next save.
//
// For tooling such as an editor. Deciding when to call it is the caller's
// policy, and a shipped build never should.
HealResult Heal(const std::filesystem::path& path, const core::Value& base);

} // namespace amanuensis::calamus
