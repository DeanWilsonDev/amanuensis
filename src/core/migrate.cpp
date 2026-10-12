#include "amanuensis/core/migrate.hpp"

#include <exception>
#include <string>

namespace amanuensis::core {

MigrateResult Migrate(Value& value, int fromVersion, int currentVersion, const MigrationStep& step)
{
  if (fromVersion < 1) {
    return MigrateResult{
        false, "Version " + std::to_string(fromVersion) + " is invalid; versions start at 1"
    };
  }
  if (fromVersion > currentVersion) {
    return MigrateResult{
        false, "The data is version " + std::to_string(fromVersion) +
                   ", but this build only knows versions up to " + std::to_string(currentVersion)
    };
  }
  if (fromVersion < currentVersion && !step) {
    return MigrateResult{
        false, "No migration step to upgrade from version " + std::to_string(fromVersion)
    };
  }

  for (int version = fromVersion; version < currentVersion; ++version) {
    try {
      step(value, version);
    }
    catch (const std::exception& error) {
      return MigrateResult{
          false, "Migrating from version " + std::to_string(version) + " failed: " + error.what()
      };
    }
  }
  return MigrateResult{true, std::string()};
}

} // namespace amanuensis::core
