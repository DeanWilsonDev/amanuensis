#include <cimmerian/test.hpp>
#include "atomic-support.hpp"
#include "support/calamus.hpp"
#include "support/values.hpp"
#include <amanuensis/calamus/heal.hpp>
#include <amanuensis/calamus/reader.hpp>
#include <amanuensis/calamus/writer.hpp>

#include <fstream>
#include <string>

using namespace calamus_atomic_test;

DESCRIBE("Atomic objects", {
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
});
