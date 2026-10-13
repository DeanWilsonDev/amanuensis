#include <cimmerian/test.hpp>
#include "conformance-support.hpp"

#include <cstddef>
#include <string>

using namespace conformance_test;

DESCRIBE("Calamus conformance", {
  DESCRIBE("Hole 8: stale unmarked fields", {
    IT("drops an unmarked field the changed base removed, and the next save leaves it out", {
      core::Value oldBase = Json(R"({"name":"door","colour":"red"})");
      core::Value newBase = Json(R"({"name":"door"})");
      std::string written = Write(oldBase, oldBase);
      calamus::LoadResult loaded = Load(written, newBase);
      ASSERT_EQUAL(loaded.stale.size(), std::size_t{1});
      ASSERT_EQUAL(Minified(loaded.value), Minified(newBase));
      ASSERT_TRUE(Write(loaded.value, newBase).find("colour") == std::string::npos);
    });

    IT("keeps a marked field the base doesn't have as an addition", {
      std::string written =
          Write(Json(R"({"name":"door","colour":"red"})"), Json(R"({"name":"door"})"));
      calamus::LoadResult loaded = Load(written, Json(R"({"name":"door"})"));
      ASSERT_EQUAL(loaded.additions.size(), std::size_t{1});
      ASSERT_EQUAL(Minified(Get(loaded.value, "colour")), std::string("\"red\""));
    });
  });
});
