#include <cimmerian/test.hpp>
#include "conformance-support.hpp"

#include <string>

using namespace conformance_test;

DESCRIBE("Calamus conformance", {
  DESCRIBE("Hole 5: arrays of objects", {
    IT("writes them as '-' items and overrides them whole", {
      core::Value base = Json(R"({"waypoints":[{"x":0.0},{"x":1.0}]})");
      core::Value entity = Json(R"({"waypoints":[{"x":0.0},{"x":2.0}]})");
      std::string written = Write(entity, base);
      ASSERT_EQUAL(
          written.substr(written.find("\n\n") + 2),
          std::string("* waypoints:\n*   -\n*     x : 0.0\n*   -\n*     x : 2.0\n")
      );
      ASSERT_EQUAL(Minified(Load(written, base).value), Minified(entity));
    });

    IT("ignores marks inside an array block, which the writer regenerates", {
      core::Value base = Json(R"({"waypoints":[{"x":0.0}]})");
      std::string edited = "@version 1\n@type Door\n\n  waypoints:\n    -\n*     x : 0.0\n";
      calamus::LoadResult loaded = Load(edited, base);
      ASSERT_TRUE(loaded.overrides.empty());
      ASSERT_EQUAL(Heal(edited, base), Write(base, base));
    });
  });
});
