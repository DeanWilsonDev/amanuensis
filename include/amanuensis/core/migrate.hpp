#pragma once

#include "amanuensis/core/value.hpp"

#include <functional>
#include <string>

namespace amanuensis::core {

struct MigrateResult {
  bool succeeded;
  std::string error;
};

// Upgrades value from fromVersion to fromVersion + 1, and throws to fail.
using MigrationStep = std::function<void(Value& value, int fromVersion)>;

// value is left part-migrated after a failed step.
MigrateResult Migrate(Value& value, int fromVersion, int currentVersion, const MigrationStep& step);

} // namespace amanuensis::core
