#pragma once

#include "support/values.hpp"
#include <amanuensis/calamus/migrate.hpp>
#include <amanuensis/calamus/reader.hpp>
#include <amanuensis/serialization/serialization.hpp>

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <numbers>
#include <sstream>
#include <stdexcept>
#include <string>

namespace calamus = amanuensis::calamus;

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

inline void AmanuensisMigrate(Door*, core::Value& value, int fromVersion)
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

inline void AmanuensisMigrate(Fragile*, core::Value&, int)
{
  throw std::runtime_error("no idea what version 1 meant");
}

inline std::string Fields(const calamus::Document& document)
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

inline calamus::Migration DoorMigration()
{
  return calamus::Migration{amanuensis::CurrentVersion<Door>, amanuensis::MigrateStep<Door>};
}

inline const std::string VersionOneDoor = "@version 1\n"
                                          "@type Door\n"
                                          "@base_hash 0123456789abcdef\n"
                                          "\n"
                                          "* pos:\n"
                                          "*   x : 4.0 | 0.0\n"
                                          "    y : 0.0\n"
                                          "    z : 0.0\n"
                                          "* angleDegrees : 90.0 | 0.0\n"
                                          "  name : \"door\"\n";

inline std::filesystem::path TestFile()
{
  return std::filesystem::temp_directory_path() / "amanuensis_migration_test.calamus";
}

inline std::string Saved()
{
  std::ifstream file(TestFile(), std::ios::binary);
  std::ostringstream content;
  content << file.rdbuf();
  return content.str();
}

} // namespace migration_test
