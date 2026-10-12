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
  bool rewritten;
  std::optional<int> migratedFrom;
  LoadResult load;
  // A parse error has a line and column. A file or write error has line 0.
  core::ParseError error;
};

// Writes the file back only when loading it finds a person's edit and the
// regenerated text differs from the text on disk. A change to the base alone
// never rewrites the file; it waits for the next save.
//
// options must be the ones the file is saved with. Without its atomic paths,
// healing would drop the marks on an atomic object's unchanged components
// and rewrite the file. For atomic objects in plain struct fields,
// AtomicPaths of the base's C++ object gives the same paths as the entity's.
//
// A migrated file is never rewritten here; it takes the current version the
// next time it's saved.
HealResult Heal(
    const std::filesystem::path& path,
    const core::Value& base,
    const WriterOptions& options = {},
    const std::optional<Migration>& migration = std::nullopt
);

} // namespace amanuensis::calamus
