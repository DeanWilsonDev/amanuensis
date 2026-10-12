#include <cimmerian/test.hpp>
#include "migration-support.hpp"
#include "support/values.hpp"
#include <amanuensis/calamus/migrate.hpp>
#include <amanuensis/calamus/reader.hpp>
#include <amanuensis/calamus/writer.hpp>
#include <amanuensis/serialization/serialization.hpp>

#include <cstddef>
#include <numbers>
#include <string>

using namespace migration_test;

DESCRIBE("Migration", {
  DESCRIBE("Calamus documents", {
    IT("migrates the overrides and drops the unmarked fields and recorded defaults", {
      auto read = calamus::Reader::ParseString(VersionOneDoor);
      REQUIRE_TRUE(read.succeeded);
      ASSERT_EQUAL(Fields(read.document), std::string("pos.x*| pos.y pos.z angleDegrees*| name"));

      auto result = calamus::Migrate(read.document, DoorMigration());
      REQUIRE_TRUE(result.succeeded);
      ASSERT_EQUAL(read.document.header.version, 3);
      ASSERT_EQUAL(Fields(read.document), std::string("position.x* angle*"));
      ASSERT_EQUAL(
          Minified(read.document.body),
          std::string(R"({"position":{"x":4.0},"angle":1.5707963267948966})")
      );
    });

    IT("loads the migrated overrides over the current base", {
      core::Value base = amanuensis::ToValue(Door{});
      auto read = calamus::Reader::ParseString(VersionOneDoor);
      REQUIRE_TRUE(calamus::Migrate(read.document, DoorMigration()).succeeded);
      calamus::LoadResult loaded = calamus::Reader::Load(read.document, base);
      ASSERT_FALSE(loaded.needsRewrite);
      ASSERT_EQUAL(
          Minified(loaded.value),
          std::string(
              R"({"position":{"x":4.0,"y":0.0,"z":0.0},"angle":1.5707963267948966,"name":"door"})"
          )
      );

      Door door = amanuensis::FromValue<Door>(loaded.value);
      ASSERT_EQUAL(door.position.x, 4.0);
      ASSERT_EQUAL(door.angle, std::numbers::pi / 2.0);

      std::string saved =
          calamus::Writer::WriteToString(read.document.header, loaded.value, base).text;
      ASSERT_TRUE(saved.starts_with("@version 3\n"));
      auto reread = calamus::Reader::ParseString(saved);
      ASSERT_EQUAL(
          calamus::Reader::Load(reread.document, base).value.Get("angle").AsDouble(), door.angle
      );
    });

    IT("leaves a document at the current version alone", {
      std::string current = VersionOneDoor;
      current.replace(0, 10, "@version 3");
      auto read = calamus::Reader::ParseString(current);
      REQUIRE_TRUE(calamus::Migrate(read.document, DoorMigration()).succeeded);
      ASSERT_EQUAL(Fields(read.document), std::string("pos.x*| pos.y pos.z angleDegrees*| name"));
    });

    IT("refuses a file newer than the code and leaves the document alone", {
      std::string newer = VersionOneDoor;
      newer.replace(0, 10, "@version 4");
      auto read = calamus::Reader::ParseString(newer);
      auto result = calamus::Migrate(read.document, DoorMigration());
      ASSERT_FALSE(result.succeeded);
      ASSERT_EQUAL(
          result.error,
          std::string(
              "@type Door: The data is version 4, but this build only knows versions up to 3"
          )
      );
      ASSERT_EQUAL(read.document.header.version, 4);
      ASSERT_EQUAL(read.document.fields.size(), std::size_t{5});
    });
  });
});
