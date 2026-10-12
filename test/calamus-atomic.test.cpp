#include <cimmerian/test.hpp>
#include <amanuensis/calamus/heal.hpp>
#include <amanuensis/calamus/reader.hpp>
#include <amanuensis/calamus/writer.hpp>
#include <amanuensis/json/reader.hpp>
#include <amanuensis/json/writer.hpp>
#include <amanuensis/serialization/serialization.hpp>

#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace calamus = amanuensis::calamus;
namespace core = amanuensis::core;

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

using atomic_test::Colour;
using atomic_test::Door;
using atomic_test::Quaternion;

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

static std::string Paths(const std::vector<core::KeyPath>& paths)
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

static calamus::Header DoorHeader()
{
  calamus::Header header;
  header.version = 1;
  header.type = "Door";
  return header;
}

static calamus::WriterOptions Atomic(std::vector<core::KeyPath> paths)
{
  calamus::WriterOptions options;
  options.atomic = std::move(paths);
  return options;
}

// The body: the text after the header and its blank line.
static std::string BodyOf(
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

static calamus::LoadResult Load(const std::string& text, const core::Value& base)
{
  return calamus::Reader::Load(calamus::Reader::ParseString(text).document, base);
}

static std::filesystem::path TestFile()
{
  return std::filesystem::temp_directory_path() / "amanuensis_atomic_test.calamus";
}

static std::string Saved()
{
  std::ifstream file(TestFile(), std::ios::binary);
  std::ostringstream content;
  content << file.rdbuf();
  return content.str();
}

static const core::Value Upright = Json(R"({"orientation":{"x":0.0,"y":0.0,"z":0.0,"w":1.0}})");
static const core::Value Turned =
    Json(R"({"orientation":{"x":0.0,"y":0.0,"z":-0.069125,"w":0.997608}})");

DESCRIBE("Atomic objects", {
  DESCRIBE("AMANUENSIS_ATOMIC", {
    IT("declares a type atomic and no other", {
      ASSERT_TRUE(amanuensis::IsAtomic<Quaternion>);
      ASSERT_TRUE(amanuensis::IsAtomic<Colour>);
      ASSERT_FALSE(amanuensis::IsAtomic<atomic_test::Vec3>);
      ASSERT_FALSE(amanuensis::IsAtomic<double>);
      ASSERT_FALSE(amanuensis::IsAtomic<std::optional<Quaternion>>);
      ASSERT_FALSE(amanuensis::IsAtomic<std::vector<Quaternion>>);
    });
  });

  DESCRIBE("AtomicPaths", {
    IT("finds atomic objects through fields, Serialise members, optionals and maps", {
      Door door;
      door.highlight = Colour{};
      door.sockets["hinge"] = Quaternion{};
      door.sockets["handle"] = Quaternion{};
      door.keyframes.push_back(Quaternion{});
      ASSERT_EQUAL(
          Paths(amanuensis::AtomicPaths(door)),
          std::string("[orientation] [light.tint] [highlight] [sockets.handle] [sockets.hinge]")
      );
    });

    IT("skips an empty optional and never looks inside arrays", {
      ASSERT_EQUAL(
          Paths(amanuensis::AtomicPaths(Door{})), std::string("[orientation] [light.tint]")
      );
    });

    IT("gives the root path for an atomic value",
       { ASSERT_EQUAL(Paths(amanuensis::AtomicPaths(Quaternion{})), std::string("[]")); });

    IT("puts each path where ToValue puts the object", {
      Door door;
      door.highlight = Colour{};
      door.sockets["hinge"] = Quaternion{};
      core::Value value = amanuensis::ToValue(door);
      for (const core::KeyPath& path : amanuensis::AtomicPaths(door)) {
        const core::Value* current = &value;
        for (const std::string& key : path) {
          current = &std::get<core::OrderedMap<core::Value>>(current->data).Get(key);
        }
        ASSERT_TRUE(std::holds_alternative<core::OrderedMap<core::Value>>(current->data));
      }
    });
  });

  DESCRIBE("Writing", {
    IT("marks every component when one differs, each with its recorded default", {
      ASSERT_EQUAL(
          BodyOf(Turned, Upright, Atomic({{"orientation"}})), std::string(
                                                                  "* orientation:\n"
                                                                  "*   x : 0.0 | 0.0\n"
                                                                  "*   y : 0.0 | 0.0\n"
                                                                  "*   z : -0.069125 | 0.0\n"
                                                                  "*   w : 0.997608 | 1.0\n"
                                                              )
      );
    });

    IT("marks only the changed components of an object that isn't atomic", {
      ASSERT_EQUAL(
          BodyOf(Turned, Upright), std::string(
                                       "* orientation:\n"
                                       "    x : 0.0\n"
                                       "    y : 0.0\n"
                                       "*   z : -0.069125 | 0.0\n"
                                       "*   w : 0.997608 | 1.0\n"
                                   )
      );
    });

    IT("leaves an unchanged atomic object unmarked", {
      ASSERT_EQUAL(
          BodyOf(Upright, Upright, Atomic({{"orientation"}})),
          std::string("  orientation:\n    x : 0.0\n    y : 0.0\n    z : 0.0\n    w : 1.0\n")
      );
    });

    IT("marks every line at any depth inside an atomic object", {
      core::Value base = Json(R"({"light":{"intensity":1.0,"colour":{"r":1.0,"g":1.0}}})");
      core::Value entity = Json(R"({"light":{"intensity":1.0,"colour":{"r":0.5,"g":1.0}}})");
      ASSERT_EQUAL(
          BodyOf(entity, base, Atomic({{"light"}})), std::string(
                                                         "* light:\n"
                                                         "*   intensity : 1.0 | 1.0\n"
                                                         "*   colour:\n"
                                                         "*     r : 0.5 | 1.0\n"
                                                         "*     g : 1.0 | 1.0\n"
                                                     )
      );
    });

    IT("ignores a change that only removes a key the base has", {
      core::Value base = Json(R"({"colour":{"r":1.0,"g":1.0,"b":1.0}})");
      core::Value entity = Json(R"({"colour":{"r":1.0,"g":1.0}})");
      ASSERT_EQUAL(
          BodyOf(entity, base, Atomic({{"colour"}})),
          std::string("  colour:\n    r : 1.0\n    g : 1.0\n")
      );
    });

    IT("ignores paths where the entity has no Object", {
      core::Value entity = Json(R"({"tags":["a"],"x":1})");
      ASSERT_EQUAL(
          BodyOf(entity, Json(R"({"tags":["b"],"x":1})"), Atomic({{"tags"}, {"x"}, {"missing"}})),
          std::string("* tags : [\"a\"] | [\"b\"]\n  x : 1\n")
      );
    });

    IT("works from the C++ types end to end", {
      Door prefab;
      Door door;
      door.orientation.z = -0.069125;
      door.light.colour.g = 0.5;
      calamus::WriterOptions options;
      options.atomic = amanuensis::AtomicPaths(door);
      std::string body = BodyOf(amanuensis::ToValue(door), amanuensis::ToValue(prefab), options);
      ASSERT_TRUE(
          body.find("*   x : 0.0 | 0.0\n*   y : 0.0 | 0.0\n*   z : -0.069125 | 0.0\n") !=
          std::string::npos
      );
      ASSERT_TRUE(
          body.find("*   tint:\n*     r : 1.0 | 1.0\n*     g : 0.5 | 1.0\n") != std::string::npos
      );
      ASSERT_TRUE(body.find("  position:\n    x : 0.0\n") != std::string::npos);
    });
  });

  DESCRIBE("Loading", {
    IT("keeps the whole rotation when the prefab turns, where a plain object mixes them", {
      std::string atomicText =
          calamus::Writer::WriteToString(DoorHeader(), Turned, Upright, Atomic({{"orientation"}}))
              .text;
      std::string plainText = calamus::Writer::WriteToString(DoorHeader(), Turned, Upright).text;
      core::Value newPrefab = Json(R"({"orientation":{"x":0.5,"y":0.0,"z":0.0,"w":0.866025}})");

      ASSERT_EQUAL(
          Minified(Load(atomicText, newPrefab).value),
          std::string(R"({"orientation":{"x":0.0,"y":0.0,"z":-0.069125,"w":0.997608}})")
      );
      ASSERT_EQUAL(
          Minified(Load(plainText, newPrefab).value),
          std::string(R"({"orientation":{"x":0.5,"y":0.0,"z":-0.069125,"w":0.997608}})")
      );
    });

    IT("reads the unchanged components as reset, which regenerates the same text", {
      calamus::WriterOptions options = Atomic({{"orientation"}});
      std::string text =
          calamus::Writer::WriteToString(DoorHeader(), Turned, Upright, options).text;
      calamus::LoadResult loaded = Load(text, Upright);
      ASSERT_TRUE(loaded.needsRewrite);
      ASSERT_EQUAL(Paths(loaded.reset), std::string("[orientation.x] [orientation.y]"));
      ASSERT_EQUAL(
          calamus::Writer::WriteToString(DoorHeader(), loaded.value, Upright, options).text, text
      );
    });

    IT("heals without rewriting when given the atomic paths", {
      calamus::WriterOptions options = Atomic({{"orientation"}});
      std::string text =
          calamus::Writer::WriteToString(DoorHeader(), Turned, Upright, options).text;
      {
        std::ofstream file(TestFile(), std::ios::binary | std::ios::trunc);
        file << text;
      }
      auto healed = calamus::Heal(TestFile(), Upright, options);
      REQUIRE_TRUE(healed.succeeded);
      ASSERT_FALSE(healed.rewritten);
      ASSERT_EQUAL(Saved(), text);

      auto withoutPaths = calamus::Heal(TestFile(), Upright);
      REQUIRE_TRUE(withoutPaths.succeeded);
      ASSERT_TRUE(withoutPaths.rewritten);
      ASSERT_TRUE(Saved().find("\n    x : 0.0\n") != std::string::npos);
    });
  });

  AFTER_ALL({ std::filesystem::remove(TestFile()); });
});
