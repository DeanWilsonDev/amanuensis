#pragma once

#include "amanuensis/calamus/migrate.hpp"
#include "amanuensis/calamus/reader.hpp"
#include "amanuensis/calamus/writer-options.hpp"
#include "amanuensis/core/parse-error.hpp"
#include "amanuensis/core/value.hpp"

#include <filesystem>
#include <optional>

namespace amanuensis::calamus {

struct HealResult {
  bool succeeded;
  // The file's text changed, so it was written back.
  bool rewritten;
  // The version the file was migrated from, when it was older than the code.
  std::optional<int> migratedFrom;
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
// options must be the ones the file is saved with. Without its atomic paths,
// healing would drop the marks on an atomic object's unchanged components
// and rewrite the file. For atomic objects in plain struct fields,
// AtomicPaths of the base's C++ object gives the same paths as the entity's.
//
// With a migration, a file older than the code is migrated before it's
// loaded, but never rewritten: it takes the current version the next time
// it's saved. A file newer than the code fails. Without one, the file is
// loaded as written.
//
// For tooling such as an editor. Deciding when to call it is the caller's
// policy, and a shipped build never should.
HealResult Heal(
    const std::filesystem::path& path,
    const core::Value& base,
    const WriterOptions& options = {},
    const std::optional<Migration>& migration = std::nullopt
);

} // namespace amanuensis::calamus
