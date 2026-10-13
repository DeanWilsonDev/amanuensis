#pragma once

#include "support/calamus.hpp"
#include "support/values.hpp"
#include <amanuensis/calamus/writer.hpp>
#include <amanuensis/serialization/serialization.hpp>

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace atomic_test {

struct Quaternion {
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
  double w = 1.0;
};
AMANUENSIS_SERIALISABLE(Quaternion, x, y, z, w);
AMANUENSIS_ATOMIC(Quaternion);

struct Colour {
  double r = 1.0;
  double g = 1.0;
  double b = 1.0;
};
AMANUENSIS_SERIALISABLE(Colour, r, g, b);
AMANUENSIS_ATOMIC(Colour);

struct Vec3 {
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
};
AMANUENSIS_SERIALISABLE(Vec3, x, y, z);

struct Light {
  double intensity = 1.0;
  Colour colour;

  template <typename Archive> void Serialise(Archive& archive)
  {
    archive.Field("intensity", intensity);
    archive.Field("tint", colour);
  }
};

struct Door {
  Vec3 position;
  Quaternion orientation;
  Light light;
  std::optional<Colour> highlight;
  std::map<std::string, Quaternion> sockets;
  std::vector<Quaternion> keyframes;
};
AMANUENSIS_SERIALISABLE(Door, position, orientation, light, highlight, sockets, keyframes);

} // namespace atomic_test

namespace calamus_atomic_test {

using atomic_test::Colour;
using atomic_test::Door;
using atomic_test::Quaternion;

inline std::string Paths(const std::vector<core::KeyPath>& paths)
{
  std::string joined;
  for (const core::KeyPath& path : paths) {
    joined += joined.empty() ? "" : " ";
    joined += "[";
    for (std::size_t index = 0; index < path.size(); ++index) {
      joined += (index == 0 ? "" : ".") + path[index];
    }
    joined += "]";
  }
  return joined;
}

inline calamus::WriterOptions Atomic(std::vector<core::KeyPath> paths)
{
  calamus::WriterOptions options;
  options.atomic = std::move(paths);
  return options;
}

inline std::string BodyOf(
    const core::Value& entity,
    const core::Value& base,
    const calamus::WriterOptions& options = {}
)
{
  auto result = calamus::Writer::WriteToString(DoorHeader(), entity, base, options);
  if (!result.succeeded) {
    return "error: " + result.error;
  }
  return result.text.substr(result.text.find("\n\n") + 2);
}

inline std::filesystem::path TestFile()
{
  return std::filesystem::temp_directory_path() / "amanuensis_atomic_test.calamus";
}

inline std::string Saved()
{
  std::ifstream file(TestFile(), std::ios::binary);
  std::ostringstream content;
  content << file.rdbuf();
  return content.str();
}

inline const core::Value Upright = Json(R"({"orientation":{"x":0.0,"y":0.0,"z":0.0,"w":1.0}})");
inline const core::Value Turned =
    Json(R"({"orientation":{"x":0.0,"y":0.0,"z":-0.069125,"w":0.997608}})");

} // namespace calamus_atomic_test
