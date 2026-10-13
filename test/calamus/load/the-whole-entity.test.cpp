#include <cimmerian/test.hpp>
#include "load-support.hpp"

#include <string>
#include <variant>

using namespace calamus_load_test;

DESCRIBE("Calamus load", {
  DESCRIBE("The whole entity", {
    IT("takes the keys the file doesn't mention from the base", {
      auto result = Load(Unchanged(Door), Door);
      ASSERT_EQUAL(Minified(result.value), Minified(Door));
      ASSERT_FALSE(result.needsRewrite);
    });

    IT("compares a float field with its base through the canonical text", {
      core::Value base = FloatBase();
      auto result = Load(Unchanged(base) + "  x : 0.1\n", base);
      ASSERT_TRUE(result.baseHashMatches);
      ASSERT_EQUAL(Paths(result.promoted), std::string(""));
      ASSERT_FALSE(result.needsRewrite);
      // The field follows the base, so it keeps the base's exact double.
      const auto& loaded = std::get<core::OrderedMap<core::Value>>(result.value.data);
      ASSERT_EQUAL(Bits(std::get<double>(loaded.Get("x").data)), Bits(static_cast<double>(0.1f)));
    });

    IT("hashes the canonical base, not the bit-exact one", {
      core::Value base = FloatBase();
      std::string bitExact = "@version 1\n@type Door\n" + HashLine(core::StableHash(base)) + "\n";
      ASSERT_FALSE(Load(bitExact, base).baseHashMatches);
      ASSERT_TRUE(Load(Unchanged(base), base).baseHashMatches);
    });

    IT("treats an empty block over a base object as no change", {
      auto result = Load(Unchanged(Door) + "* position:\n", Door);
      ASSERT_EQUAL(Paths(result.reset), std::string("position"));
      ASSERT_EQUAL(Minified(result.value), Minified(Door));
    });

    IT("lets an empty block override a base value that isn't an object", {
      core::Value base = Json(R"({"light":null})");
      auto result = Load(Unchanged(base) + "* light:\n", base);
      ASSERT_EQUAL(Paths(result.overrides), std::string("light"));
      ASSERT_EQUAL(Minified(result.value), std::string(R"({"light":{}})"));
    });

    IT("lets fields under a block replace a base value that isn't an object", {
      core::Value base = Json(R"({"light":null})");
      auto result = Load(Unchanged(base) + "* light:\n*   intensity : 2.0\n", base);
      ASSERT_EQUAL(Paths(result.additions), std::string("light.intensity"));
      ASSERT_EQUAL(Minified(result.value), std::string(R"({"light":{"intensity":2.0}})"));
    });

    IT("never treats an Integer as equal to a Double", {
      auto result = Load(Unchanged(Door) + "  position:\n    x : 0\n", Door);
      ASSERT_EQUAL(Paths(result.promoted), std::string("position.x"));
    });
  });
});
