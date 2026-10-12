#include <cimmerian/test.hpp>
#include "support/calamus.hpp"

#include <string>

DESCRIBE("Calamus conformance", {
  DESCRIBE("Hole 7: separators inside strings", {
    IT("reads ',', ':', '|', '#' and quotes inside strings as characters", {
      core::Value entity =
          Json(R"({"s":"a, b: c | d # e \"f\" \\ g","list":["x, y", "| z", "#"]})");
      core::Value base = Json(R"({"s":"| old # value","list":[]})");
      std::string written = Write(entity, base);
      calamus::LoadResult loaded = Load(written, base);
      ASSERT_EQUAL(Minified(loaded.value), Minified(entity));
      ASSERT_TRUE(loaded.moved.empty());
    });
  });
});
