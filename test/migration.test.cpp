#include <cimmerian/test.hpp>
#include <amanuensis/calamus/heal.hpp>
#include <amanuensis/calamus/migrate.hpp>
#include <amanuensis/calamus/reader.hpp>
#include <amanuensis/calamus/writer.hpp>
#include <amanuensis/core/migrate.hpp>
#include <amanuensis/json/reader.hpp>
#include <amanuensis/json/writer.hpp>
#include <amanuensis/serialization/serialization.hpp>

#include <filesystem>
#include <fstream>
#include <numbers>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace calamus = amanuensis::calamus;
namespace core = amanuensis::core;

namespace migration_test {

struct Vec3 {
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
};
AMANUENSIS_SERIALISABLE(Vec3, x, y, z);

// Version 1 had "pos" and "angleDegrees". Version 2 renamed "pos" to
// "position", and version 3 replaced "angleDegrees" with "angle" in radians.
struct Door {
  Vec3 position;
  double angle = 0.0;
  std::string name = "door";
};
AMANUENSIS_SERIALISABLE(Door, position, angle, name);
AMANUENSIS_VERSION(Door, 3);

void AmanuensisMigrate(Door*, core::Value& value, int fromVersion)
{
  if (fromVersion == 1 && value.Contains("pos")) {
    value.Insert("position", value.Get("pos"));
    value.Erase("pos");
  }
  if (fromVersion == 2 && value.Contains("angleDegrees")) {
    double degrees = value.Get("angleDegrees").AsDouble();
    value.Insert("angle", core::Value{degrees * std::numbers::pi / 180.0});
    value.Erase("angleDegrees");
  }
}

struct Plain {
  int count = 0;
};
AMANUENSIS_SERIALISABLE(Plain, count);

struct Fragile {
  int count = 0;
};
AMANUENSIS_SERIALISABLE(Fragile, count);
AMANUENSIS_VERSION(Fragile, 2);

void AmanuensisMigrate(Fragile*, core::Value&, int)
{
  throw std::runtime_error("no idea what version 1 meant");
}

} // namespace migration_test

using migration_test::Door;
using migration_test::Fragile;
using migration_test::Plain;

static core::Value Json(std::string_view text)
{
  return amanuensis::json::Reader::ParseString(text).value;
}

static std::string Minified(const core::Value& value)
{
  amanuensis::json::WriterOptions options;
  options.pretty = false;
  options.trailingNewline = false;
  return amanuensis::json::Writer::WriteToString(value, options);
}

static std::string Fields(const calamus::Document& document)
{
  std::string fields;
  for (const calamus::Field& field : document.fields) {
    fields += fields.empty() ? "" : " ";
    for (std::size_t index = 0; index < field.path.size(); ++index) {
      fields += (index == 0 ? "" : ".") + field.path[index];
    }
    fields += field.marked ? "*" : "";
    fields += field.recordedDefault ? "|" : "";
  }
  return fields;
}

static calamus::Migration DoorMigration()
{
  return calamus::Migration{amanuensis::CurrentVersion<Door>, amanuensis::MigrateStep<Door>};
}

static const std::string VersionOneDoor = "@version 1\n"
                                          "@type Door\n"
                                          "@base_hash 0123456789abcdef\n"
                                          "\n"
                                          "* pos:\n"
                                          "*   x : 4.0 | 0.0\n"
                                          "    y : 0.0\n"
                                          "    z : 0.0\n"
                                          "* angleDegrees : 90.0 | 0.0\n"
                                          "  name : \"door\"\n";

static std::filesystem::path TestFile()
{
  return std::filesystem::temp_directory_path() / "amanuensis_migration_test.calamus";
}

static std::string Saved()
{
  std::ifstream file(TestFile(), std::ios::binary);
  std::ostringstream content;
  content << file.rdbuf();
  return content.str();
}

DESCRIBE("Migration", {
  DESCRIBE("Value::Erase", {
    IT("removes a key and keeps the rest in order", {
      core::Value object = Json(R"({"a":1,"b":2,"c":3,"d":4})");
      ASSERT_TRUE(object.Erase("b"));
      ASSERT_EQUAL(Minified(object), std::string(R"({"a":1,"c":3,"d":4})"));
      ASSERT_EQUAL(object.Get("d").AsInteger(), 4LL);
      object.Insert("b", core::Value{5LL});
      ASSERT_EQUAL(Minified(object), std::string(R"({"a":1,"c":3,"d":4,"b":5})"));
    });

    IT("says when the key wasn't there", {
      core::Value object = Json(R"({"a":1})");
      ASSERT_FALSE(object.Erase("missing"));
      ASSERT_EQUAL(object.Size(), std::size_t{1});
    });

    IT("throws on a value that isn't an Object", {
      core::Value array = core::Value::MakeArray();
      ASSERT_THROWS(array.Erase("a"), core::TypeMismatchError);
    });
  });

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

      // The next save writes the current version, which loads with nothing to
      // migrate.
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

  AFTER_ALL({ std::filesystem::remove(TestFile()); });
});
