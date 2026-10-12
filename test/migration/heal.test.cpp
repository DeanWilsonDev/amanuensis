#include <cimmerian/test.hpp>
#include "migration-support.hpp"
#include <amanuensis/calamus/heal.hpp>
#include <amanuensis/serialization/serialization.hpp>

#include <fstream>
#include <numbers>
#include <string>

using namespace migration_test;

DESCRIBE("Migration", {
  DESCRIBE("Heal", {
    IT("migrates an old file in memory but leaves it on disk until the next save", {
      {
        std::ofstream file(TestFile(), std::ios::binary | std::ios::trunc);
        file << VersionOneDoor;
      }
      auto healed = calamus::Heal(TestFile(), amanuensis::ToValue(Door{}), {}, DoorMigration());
      REQUIRE_TRUE(healed.succeeded);
      ASSERT_FALSE(healed.rewritten);
      REQUIRE_TRUE(healed.migratedFrom.has_value());
      ASSERT_EQUAL(*healed.migratedFrom, 1);
      ASSERT_EQUAL(healed.load.value.Get("angle").AsDouble(), std::numbers::pi / 2.0);
      ASSERT_EQUAL(Saved(), VersionOneDoor);
    });

    IT("fails on a file newer than the code", {
      std::string newer = VersionOneDoor;
      newer.replace(0, 10, "@version 9");
      {
        std::ofstream file(TestFile(), std::ios::binary | std::ios::trunc);
        file << newer;
      }
      auto healed = calamus::Heal(TestFile(), amanuensis::ToValue(Door{}), {}, DoorMigration());
      ASSERT_FALSE(healed.succeeded);
      ASSERT_TRUE(healed.error.message.find("version 9") != std::string::npos);
      ASSERT_EQUAL(Saved(), newer);
    });
  });
});
