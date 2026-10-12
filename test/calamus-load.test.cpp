#include <cimmerian/test.hpp>
#include "calamus/canonical.hpp"
#include <amanuensis/calamus/reader.hpp>
#include <amanuensis/core/stable-hash.hpp>
#include <amanuensis/json/reader.hpp>
#include <amanuensis/json/writer.hpp>

#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace calamus = amanuensis::calamus;
namespace core = amanuensis::core;

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

static std::string HashLine(std::uint64_t hash)
{
  char buffer[40];
  std::snprintf(
      buffer, sizeof(buffer), "@base_hash %016llx\n", static_cast<unsigned long long>(hash)
  );
  return buffer;
}

// A header whose @base_hash matches base, so the base hasn't changed.
static std::string Unchanged(const core::Value& base)
{
  return "@version 1\n@type Door\n" + HashLine(core::StableHash(calamus::Canonicalise(base))) +
         "\n";
}

// A header whose @base_hash matches some other base, so the base has changed.
static const std::string Changed = "@version 1\n@type Door\n@base_hash 0123456789abcdef\n\n";

static const std::string Unhashed = "@version 1\n@type Door\n\n";

// Loads text over base. A parse error comes back as the loaded value, so a
// test that compares the value shows it.
static calamus::LoadResult Load(const std::string& text, const core::Value& base)
{
  auto read = calamus::Reader::ParseString(text);
  if (!read.succeeded) {
    calamus::LoadResult failed;
    failed.value = core::Value{"parse error: " + read.error.message};
    return failed;
  }
  return calamus::Reader::Load(read.document, base);
}

// Each path dotted, separated by spaces.
static std::string Paths(const std::vector<core::KeyPath>& paths)
{
  std::string joined;
  for (const core::KeyPath& path : paths) {
    if (!joined.empty()) {
      joined += " ";
    }
    for (std::size_t index = 0; index < path.size(); ++index) {
      joined += (index == 0 ? "" : ".") + path[index];
    }
  }
  return joined;
}

static std::string Canonical(double value)
{
  std::string output;
  calamus::AppendCanonicalDouble(output, value);
  return output;
}

static std::uint64_t Bits(double value)
{
  return std::bit_cast<std::uint64_t>(value);
}

// {"x": 0.1f}, as a float field's ToValue gives it.
static core::Value FloatBase()
{
  core::Value base = Json(R"({"x":0.0})");
  std::get<core::OrderedMap<core::Value>>(base.data).Insert(
      "x", core::Value{static_cast<double>(0.1f)}
  );
  return base;
}

static const core::Value Door =
    Json(R"({"position":{"x":0.0,"y":0.0,"z":0.0},"name":"door","locked":false,"tags":["a"]})");

DESCRIBE("Calamus load", {
  DESCRIBE("Marked fields", {
    IT("override the base when they differ", {
      auto result = Load(
          Unchanged(Door) + "* position:\n*   x : 4.5 | 0.0\n    y : 0.0\n    z : 0.0\n", Door
      );
      ASSERT_EQUAL(
          Minified(result.value),
          std::string(
              R"({"position":{"x":4.5,"y":0.0,"z":0.0},"name":"door","locked":false,"tags":["a"]})"
          )
      );
      ASSERT_EQUAL(Paths(result.overrides), std::string("position.x"));
      ASSERT_FALSE(result.needsRewrite);
      ASSERT_EQUAL(Paths(result.moved), std::string(""));
    });

    IT("replace arrays whole", {
      auto result = Load(Unchanged(Door) + "* tags : [\"b\", \"c\"]\n", Door);
      ASSERT_EQUAL(
          Minified(result.value), std::string(
                                      R"({"position":{"x":0.0,"y":0.0,"z":0.0},)"
                                      R"("name":"door","locked":false,"tags":["b","c"]})"
                                  )
      );
    });

    IT("keep a field the base doesn't have as an addition, at the end of its object", {
      auto result = Load(Unchanged(Door) + "* position:\n*   w : 1.0\n", Door);
      ASSERT_EQUAL(
          Minified(result.value), std::string(
                                      R"({"position":{"x":0.0,"y":0.0,"z":0.0,"w":1.0},)"
                                      R"("name":"door","locked":false,"tags":["a"]})"
                                  )
      );
      ASSERT_EQUAL(Paths(result.additions), std::string("position.w"));
      ASSERT_EQUAL(Paths(result.overrides), std::string("position.w"));
      ASSERT_FALSE(result.needsRewrite);
    });

    IT("follow an unchanged base they equal, and ask for a rewrite", {
      auto result = Load(Unchanged(Door) + "* locked : false | false\n", Door);
      ASSERT_EQUAL(Paths(result.reset), std::string("locked"));
      ASSERT_EQUAL(Paths(result.overrides), std::string(""));
      ASSERT_TRUE(result.needsRewrite);
      ASSERT_EQUAL(Minified(result.value), Minified(Door));
    });

    IT("follow a changed base they now equal, without a rewrite", {
      auto result = Load(Changed + "* locked : false | true\n", Door);
      ASSERT_FALSE(result.baseHashMatches);
      ASSERT_EQUAL(Paths(result.reset), std::string(""));
      ASSERT_EQUAL(Paths(result.overrides), std::string(""));
      ASSERT_FALSE(result.needsRewrite);
    });

    IT("are listed as moved when the recorded default no longer matches the base", {
      auto result = Load(
          Unchanged(Door) +
              "* position:\n*   x : 4.5 | 1.0\n*   y : 2.0 | 0.0\n* name : \"gate\"\n",
          Door
      );
      ASSERT_EQUAL(Paths(result.moved), std::string("position.x"));
      ASSERT_EQUAL(Paths(result.overrides), std::string("position.x position.y name"));
    });

    IT("are listed as moved when the base dropped the recorded default's field", {
      auto result = Load(Changed + "* colour : \"red\" | \"blue\"\n", Door);
      ASSERT_EQUAL(Paths(result.additions), std::string("colour"));
      ASSERT_EQUAL(Paths(result.moved), std::string("colour"));
    });
  });

  DESCRIBE("Unmarked fields", {
    IT("follow the base when they equal it", {
      auto result = Load(Unchanged(Door) + "  name : \"door\"\n  tags : [\"a\"]\n", Door);
      ASSERT_EQUAL(Paths(result.overrides), std::string(""));
      ASSERT_EQUAL(Paths(result.promoted), std::string(""));
      ASSERT_FALSE(result.needsRewrite);
    });

    IT("are promoted when they differ from an unchanged base", {
      auto result = Load(Unchanged(Door) + "  name : \"gate\"\n", Door);
      ASSERT_EQUAL(Paths(result.promoted), std::string("name"));
      ASSERT_EQUAL(Paths(result.overrides), std::string("name"));
      ASSERT_TRUE(result.needsRewrite);
      ASSERT_EQUAL(
          Minified(result.value),
          std::string(
              R"({"position":{"x":0.0,"y":0.0,"z":0.0},"name":"gate","locked":false,"tags":["a"]})"
          )
      );
    });

    IT("are promoted when there's no @base_hash", {
      auto result = Load(Unhashed + "  name : \"gate\"\n", Door);
      ASSERT_TRUE(result.baseHashMatches);
      ASSERT_EQUAL(Paths(result.promoted), std::string("name"));
      ASSERT_TRUE(result.needsRewrite);
    });

    IT("are promoted when an unchanged base doesn't have them", {
      auto result = Load(Unchanged(Door) + "  colour : \"red\"\n", Door);
      ASSERT_EQUAL(Paths(result.promoted), std::string("colour"));
      ASSERT_EQUAL(
          Minified(result.value), std::string(
                                      R"({"position":{"x":0.0,"y":0.0,"z":0.0},)"
                                      R"("name":"door","locked":false,"tags":["a"],"colour":"red"})"
                                  )
      );
    });

    IT("follow a changed base, reported as refreshed", {
      auto result = Load(Changed + "  name : \"gate\"\n", Door);
      ASSERT_EQUAL(Paths(result.refreshed), std::string("name"));
      ASSERT_EQUAL(Paths(result.overrides), std::string(""));
      ASSERT_FALSE(result.needsRewrite);
      ASSERT_EQUAL(Minified(result.value), Minified(Door));
    });

    IT("are dropped when a changed base no longer has them", {
      auto result = Load(Changed + "  colour : \"red\"\n", Door);
      ASSERT_EQUAL(Paths(result.stale), std::string("colour"));
      ASSERT_FALSE(result.needsRewrite);
      ASSERT_EQUAL(Minified(result.value), Minified(Door));
    });
  });

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

  DESCRIBE("Canonical numbers", {
    IT("writes a double that is exactly a float as the shortest float text", {
      ASSERT_EQUAL(Canonical(static_cast<double>(0.1f)), std::string("0.1"));
      ASSERT_EQUAL(Canonical(16777216.0), std::string("16777216.0"));
      ASSERT_EQUAL(Canonical(-0.0), std::string("-0.0"));
      ASSERT_EQUAL(Canonical(1.0), std::string("1.0"));
    });

    IT("writes any other double as the shortest double text", {
      ASSERT_EQUAL(Canonical(0.1), std::string("0.1"));
      ASSERT_EQUAL(Canonical(1.0 / 3.0), std::string("0.3333333333333333"));
      ASSERT_EQUAL(Canonical(1e300), std::string("1e+300"));
      ASSERT_EQUAL(Canonical(5e-324), std::string("5e-324"));
    });

    IT("reads the canonical text back as the canonical double", {
      ASSERT_EQUAL(Bits(calamus::CanonicalDouble(static_cast<double>(0.1f))), Bits(0.1));
      ASSERT_EQUAL(Bits(calamus::CanonicalDouble(0.1)), Bits(0.1));
      ASSERT_EQUAL(Bits(calamus::CanonicalDouble(-0.0)), Bits(-0.0));
      ASSERT_EQUAL(Bits(calamus::CanonicalDouble(1.0 / 3.0)), Bits(1.0 / 3.0));
    });

    IT("is idempotent", {
      for (double value :
           {static_cast<double>(0.1f), static_cast<double>(46.103531f), 0.1, 1.0 / 3.0,
            3.4028234663852886e38, 1e300, 1e-45}) {
        double once = calamus::CanonicalDouble(value);
        ASSERT_EQUAL(Bits(calamus::CanonicalDouble(once)), Bits(once));
      }
    });

    IT("replaces doubles at any depth and leaves everything else alone", {
      core::Value value = Json(R"({"a":1,"b":"s","c":[0.0,{"d":0.0}],"e":true})");
      auto& object = std::get<core::OrderedMap<core::Value>>(value.data);
      auto& array = std::get<std::vector<core::Value>>(object.Get("c").data);
      array[0] = core::Value{static_cast<double>(0.1f)};
      std::get<core::OrderedMap<core::Value>>(array[1].data)
          .Insert("d", core::Value{static_cast<double>(0.2f)});
      object.Insert("f", core::Value{std::numeric_limits<double>::quiet_NaN()});

      core::Value canonical = calamus::Canonicalise(value);
      auto& canonicalObject = std::get<core::OrderedMap<core::Value>>(canonical.data);
      ASSERT_TRUE(std::isnan(std::get<double>(canonicalObject.Get("f").data)));
      // JSON can't write NaN, so swap it out before comparing the rest.
      canonicalObject.Insert("f", core::Value{});
      ASSERT_EQUAL(
          Minified(canonical),
          std::string(R"({"a":1,"b":"s","c":[0.1,{"d":0.2}],"e":true,"f":null})")
      );
    });
  });
});
