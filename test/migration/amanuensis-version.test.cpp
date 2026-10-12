#include <cimmerian/test.hpp>
#include "migration-support.hpp"
#include "support/values.hpp"
#include <amanuensis/serialization/serialization.hpp>

#include <numbers>
#include <string>

using namespace migration_test;

DESCRIBE("Migration", {
  DESCRIBE("AMANUENSIS_VERSION", {
    IT("sets a type's current version, which is 1 without it", {
      ASSERT_EQUAL(amanuensis::CurrentVersion<Door>, 3);
      ASSERT_EQUAL(amanuensis::CurrentVersion<Plain>, 1);
    });

    IT("runs the type's steps from the data's version up", {
      core::Value value = Json(R"({"pos":{"x":4.0},"angleDegrees":90.0,"name":"gate"})");
      auto result = amanuensis::Migrate<Door>(value, 1);
      REQUIRE_TRUE(result.succeeded);
      ASSERT_EQUAL(
          Minified(value),
          std::string(R"({"name":"gate","position":{"x":4.0},"angle":1.5707963267948966})")
      );

      core::Value fromTwo = Json(R"({"angleDegrees":180.0})");
      REQUIRE_TRUE(amanuensis::Migrate<Door>(fromTwo, 2).succeeded);
      ASSERT_EQUAL(fromTwo.Get("angle").AsDouble(), std::numbers::pi);
    });

    IT("migrates whole JSON data before FromValue", {
      core::Value value =
          Json(R"({"pos":{"x":1.0,"y":2.0,"z":3.0},"angleDegrees":90.0,"name":"gate"})");
      REQUIRE_TRUE(amanuensis::Migrate<Door>(value, 1).succeeded);
      Door door = amanuensis::FromValue<Door>(value);
      ASSERT_EQUAL(door.position.y, 2.0);
      ASSERT_EQUAL(door.angle, std::numbers::pi / 2.0);
    });

    IT("needs no steps for a type at version 1, and still refuses newer data", {
      core::Value value = Json(R"({"count":1})");
      ASSERT_TRUE(amanuensis::Migrate<Plain>(value, 1).succeeded);
      ASSERT_FALSE(amanuensis::Migrate<Plain>(value, 2).succeeded);
    });

    IT("reports a step that throws", {
      core::Value value = Json(R"({"count":1})");
      auto result = amanuensis::Migrate<Fragile>(value, 1);
      ASSERT_FALSE(result.succeeded);
      ASSERT_EQUAL(
          result.error, std::string("Migrating from version 1 failed: no idea what version 1 meant")
      );
    });
  });
});
