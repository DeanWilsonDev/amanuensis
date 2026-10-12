#pragma once

#include "amanuensis/calamus/reader.hpp"
#include "amanuensis/core/migrate.hpp"

namespace amanuensis::calamus {

// How to bring an entity type's data up to date. For a C++ type declared with
// AMANUENSIS_VERSION:
//
//   calamus::Migration{amanuensis::CurrentVersion<Door>, amanuensis::MigrateStep<Door>}
struct Migration {
  int currentVersion = 1;
  core::MigrationStep step;
};

// Brings a document older than migration.currentVersion up to date, before
// Reader::Load. Its marked fields are migrated as one sparse Object and become
// the document's fields, all marked. Unmarked fields and recorded defaults are
// dropped, since they're in the old layout, so everything else follows the
// current base. header.version becomes the current version and body the
// migrated overrides. A document already at the current version is left
// alone, and on failure, including a document newer than the code, the
// document is unchanged. See docs/calamus-spec.md, "Migration".
core::MigrateResult Migrate(Document& document, const Migration& migration);

} // namespace amanuensis::calamus
