#pragma once

#include "amanuensis/core/value.hpp"

#include <functional>
#include <string>

namespace amanuensis::core {

struct MigrateResult {
  bool succeeded;
  std::string error;
};

// Upgrades value by one version, from fromVersion to fromVersion + 1. It
// throws to fail.
using MigrationStep = std::function<void(Value& value, int fromVersion)>;

// Brings value from fromVersion up to currentVersion, calling step once per
// version in order. Fails without throwing on a version below 1, on data newer
// than currentVersion, when an upgrade is needed and there's no step, or when
// a step throws. value is left part-migrated after a failed step.
MigrateResult Migrate(Value& value, int fromVersion, int currentVersion, const MigrationStep& step);

} // namespace amanuensis::core
