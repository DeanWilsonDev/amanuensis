#include <cimmerian/test.hpp>
#include "support/values.hpp"
#include <amanuensis/core/migrate.hpp>

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

DESCRIBE("Migration", {
  DESCRIBE("core::Migrate", {
    IT("calls the step once per version, in order", {
      std::vector<int> calls;
      core::Value value = core::Value::MakeObject();
      auto result = core::Migrate(value, 2, 5, [&calls](core::Value&, int fromVersion) {
        calls.push_back(fromVersion);
      });
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(calls.size(), std::size_t{3});
      ASSERT_TRUE(calls[0] == 2 && calls[1] == 3 && calls[2] == 4);
    });

    IT("leaves data at the current version alone", {
      bool called = false;
      core::Value value = core::Value::MakeObject();
      auto result = core::Migrate(value, 3, 3, [&called](core::Value&, int) { called = true; });
      ASSERT_TRUE(result.succeeded);
      ASSERT_FALSE(called);
    });

    IT("refuses data newer than the code, naming both versions", {
      core::Value value = core::Value::MakeObject();
      auto result = core::Migrate(value, 4, 3, {});
      ASSERT_FALSE(result.succeeded);
      ASSERT_EQUAL(
          result.error,
          std::string("The data is version 4, but this build only knows versions up to 3")
      );
    });

    IT("rejects a version below 1", {
      core::Value value = core::Value::MakeObject();
      auto result = core::Migrate(value, 0, 3, {});
      ASSERT_EQUAL(result.error, std::string("Version 0 is invalid; versions start at 1"));
    });

    IT("reports a failing step with its version, and a missing one", {
      core::Value value = core::Value::MakeObject();
      auto failed = core::Migrate(value, 1, 3, [](core::Value&, int fromVersion) {
        if (fromVersion == 2) {
          throw std::runtime_error("bad data");
        }
      });
      ASSERT_EQUAL(failed.error, std::string("Migrating from version 2 failed: bad data"));

      auto missing = core::Migrate(value, 1, 2, {});
      ASSERT_EQUAL(missing.error, std::string("No migration step to upgrade from version 1"));
    });
  });
});
