#pragma once

#include "amanuensis/calamus/reader.hpp"
#include "amanuensis/core/migrate.hpp"

namespace amanuensis::calamus {

// For a type declared with AMANUENSIS_VERSION:
//
//   calamus::Migration{amanuensis::CurrentVersion<Door>, amanuensis::MigrateStep<Door>}
struct Migration {
  int currentVersion = 1;
  core::MigrationStep step;
};

// Only the marked fields are migrated. Unmarked fields and recorded defaults
// are in the old layout, so they're dropped and follow the current base. On
// failure the document is unchanged.
core::MigrateResult Migrate(Document& document, const Migration& migration);

} // namespace amanuensis::calamus
