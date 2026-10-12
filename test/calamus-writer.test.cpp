#include <cimmerian/test.hpp>
#include "calamus/canonical.hpp"
#include <amanuensis/calamus/reader.hpp>
#include <amanuensis/calamus/writer.hpp>
#include <amanuensis/core/stable-hash.hpp>
#include <amanuensis/json/reader.hpp>
#include <amanuensis/json/writer.hpp>

#include <cstdio>
#include <limits>
#include <string>
#include <string_view>
#include <variant>

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

static std::string Hash(const core::Value& base)
{
  char buffer[17];
  std::snprintf(
      buffer, sizeof(buffer), "%016llx",
      static_cast<unsigned long long>(core::StableHash(calamus::Canonicalise(base)))
  );
  return buffer;
}

static calamus::Header DoorHeader()
{
  calamus::Header header;
  header.version = 1;
  header.type = "Door";
  return header;
}

static std::string Write(const core::Value& entity, const core::Value& base)
{
  auto result = calamus::Writer::WriteToString(DoorHeader(), entity, base);
  return result.succeeded ? result.text : "error: " + result.error;
}

static std::string BodyOf(const core::Value& entity, const core::Value& base)
{
  std::string text = Write(entity, base);
  std::size_t bodyStart = text.find("\n\n");
  return bodyStart == std::string::npos ? text : text.substr(bodyStart + 2);
}

static calamus::LoadResult RoundTrip(const core::Value& entity, const core::Value& base)
{
  auto written = calamus::Writer::WriteToString(DoorHeader(), entity, base);
  auto read = calamus::Reader::ParseString(written.text);
  if (!written.succeeded || !read.succeeded) {
    calamus::LoadResult failed;
    failed.value = core::Value{"failed: " + written.error + read.error.message};
    return failed;
  }
  return calamus::Reader::Load(read.document, base);
}

static std::string Canonical(const core::Value& value)
{
  return Minified(calamus::Canonicalise(value));
}

static const core::Value Prefab = Json(R"({
  "position": {"x": 0.0, "y": 0.0, "z": 0.0},
  "scale": {"x": 1.0, "y": 1.0, "z": 1.0},
  "name": "",
  "flags": 160,
  "locked": false,
  "tags": ["puzzle", "act1"],
  "waypoints": [{"x": 0.0, "z": 2.5}],
  "light": {"intensity": 1.0, "colour": {"r": 1.0, "g": 0.9, "b": 0.8}}
})");

static const core::Value Door = Json(R"({
  "position": {"x": 46.5, "y": 108.25, "z": 0.0},
  "scale": {"x": 1.0, "y": 1.0, "z": 1.0},
  "name": "",
  "flags": 160,
  "locked": true,
  "tags": ["puzzle", "act1"],
  "waypoints": [{"x": 0.0, "z": 2.5}, {"x": 4.0, "z": 2.5}],
  "light": {"intensity": 2.5, "colour": {"r": 1.0, "g": 0.9, "b": 0.8}}
})");

DESCRIBE("Calamus writer", {
  IT("writes an entity over its prefab", {
    calamus::Header header;
    header.version = 3;
    header.type = "Door";
    header.id = 0x3f9c2a7be1d04c58;
    header.base = "prefabs/wooden_door";
    auto result = calamus::Writer::WriteToString(header, Door, Prefab);
    REQUIRE_TRUE(result.succeeded);
    ASSERT_EQUAL(
        result.text, "@version 3\n"
                     "@type Door\n"
                     "@id 3f9c2a7be1d04c58\n"
                     "@base \"prefabs/wooden_door\"\n"
                     "@base_hash " +
                         Hash(Prefab) +
                         "\n"
                         "\n"
                         "* position:\n"
                         "*   x : 46.5 | 0.0\n"
                         "*   y : 108.25 | 0.0\n"
                         "    z : 0.0\n"
                         "  scale:\n"
                         "    x : 1.0\n"
                         "    y : 1.0\n"
                         "    z : 1.0\n"
                         "  name : \"\"\n"
                         "  flags : 160\n"
                         "* locked : true | false\n"
                         "  tags : [\"puzzle\", \"act1\"]\n"
                         "* waypoints:\n"
                         "*   -\n"
                         "*     x : 0.0\n"
                         "*     z : 2.5\n"
                         "*   -\n"
                         "*     x : 4.0\n"
                         "*     z : 2.5\n"
                         "* light:\n"
                         "*   intensity : 2.5 | 1.0\n"
                         "    colour:\n"
                         "      r : 1.0\n"
                         "      g : 0.9\n"
                         "      b : 0.8\n"
    );
  });

  IT("gives the same text every time", { ASSERT_EQUAL(Write(Door, Prefab), Write(Door, Prefab)); });

  DESCRIBE("Header", {
    IT("always writes @base_hash, and leaves out @id and @base when there are none", {
      ASSERT_EQUAL(
          Write(Json(R"({"x":1})"), Json(R"({"x":1})")),
          "@version 1\n@type Door\n@base_hash " + Hash(Json(R"({"x":1})")) + "\n\n  x : 1\n"
      );
    });

    IT("ignores the header's own @base_hash", {
      calamus::Header header = DoorHeader();
      header.baseHash = 0x0123456789abcdef;
      auto result = calamus::Writer::WriteToString(header, Json("{}"), Json("{}"));
      ASSERT_EQUAL(result.text, "@version 1\n@type Door\n@base_hash " + Hash(Json("{}")) + "\n");
    });

    IT("writes @id as 16 lowercase hex digits", {
      calamus::Header header = DoorHeader();
      header.id = 0x3FA;
      auto result = calamus::Writer::WriteToString(header, Json("{}"), Json("{}"));
      ASSERT_TRUE(result.text.find("\n@id 00000000000003fa\n") != std::string::npos);
    });

    IT("writes no blank line and one final newline for an empty body", {
      std::string text = Write(Json("{}"), Json("{}"));
      ASSERT_EQUAL(text.find("\n\n"), std::string::npos);
      ASSERT_EQUAL(text.back(), '\n');
    });

    IT("escapes @base", {
      calamus::Header header = DoorHeader();
      header.base = "prefabs/\"odd\"";
      auto result = calamus::Writer::WriteToString(header, Json("{}"), Json("{}"));
      ASSERT_TRUE(result.text.find("\n@base \"prefabs/\\\"odd\\\"\"\n") != std::string::npos);
    });

    IT("refuses a header the reader would reject", {
      calamus::Header header = DoorHeader();
      header.version = 0;
      auto noVersion = calamus::Writer::WriteToString(header, Json("{}"), Json("{}"));
      ASSERT_FALSE(noVersion.succeeded);
      ASSERT_EQUAL(noVersion.error, std::string("@version must be at least 1"));

      header = DoorHeader();
      header.type = "wooden door";
      auto badType = calamus::Writer::WriteToString(header, Json("{}"), Json("{}"));
      ASSERT_FALSE(badType.succeeded);
    });

    IT("refuses an entity that isn't an Object", {
      ASSERT_EQUAL(
          Write(Json("[1]"), Json("{}")), std::string("error: The entity must be an Object")
      );
    });
  });

  DESCRIBE("Marks", {
    IT("marks a field the base doesn't have, with no recorded default", {
      ASSERT_EQUAL(
          BodyOf(Json(R"({"x":1,"extra":2})"), Json(R"({"x":1})")),
          std::string("  x : 1\n* extra : 2\n")
      );
    });

    IT("leaves out keys only the base has", {
      ASSERT_EQUAL(
          BodyOf(Json(R"({"x":1})"), Json(R"({"x":1,"gone":{"y":2}})")), std::string("  x : 1\n")
      );
    });

    IT("marks a changed array whole and records the base array", {
      ASSERT_EQUAL(
          BodyOf(Json(R"({"tags":["a"]})"), Json(R"({"tags":["puzzle","act1"]})")),
          std::string("* tags : [\"a\"] | [\"puzzle\", \"act1\"]\n")
      );
    });

    IT("records no default when the base value is an object", {
      ASSERT_EQUAL(
          BodyOf(Json(R"({"light":null})"), Json(R"({"light":{"intensity":1.0}})")),
          std::string("* light : null\n")
      );
    });

    IT("marks every line under a value that changed whole", {
      ASSERT_EQUAL(
          BodyOf(
              Json(R"({"light":{"intensity":1.0,"colour":{"r":1}}})"), Json(R"({"light":null})")
          ),
          std::string("* light:\n*   intensity : 1.0\n*   colour:\n*     r : 1\n")
      );
    });

    IT("marks everything when the base isn't an Object", {
      ASSERT_EQUAL(
          BodyOf(Json(R"({"x":1,"list":[{"y":2}]})"), core::Value()),
          std::string("* x : 1\n* list:\n*   -\n*     y : 2\n")
      );
    });

    IT("doesn't mark an empty object over a base object", {
      ASSERT_EQUAL(
          BodyOf(Json(R"({"light":{}})"), Json(R"({"light":{"intensity":1.0}})")),
          std::string("  light:\n")
      );
    });

    IT("never treats an Integer as equal to a Double", {
      ASSERT_EQUAL(
          BodyOf(Json(R"({"x":1})"), Json(R"({"x":1.0})")), std::string("* x : 1 | 1.0\n")
      );
    });
  });

  DESCRIBE("Numbers", {
    IT("writes a float field without float noise, and compares it canonically", {
      core::Value entity = Json(R"({"x":0.0})");
      std::get<core::OrderedMap<core::Value>>(entity.data)
          .Insert("x", core::Value{static_cast<double>(0.1f)});
      ASSERT_EQUAL(BodyOf(entity, Json(R"({"x":0.0})")), std::string("* x : 0.1 | 0.0\n"));
      ASSERT_EQUAL(BodyOf(entity, Json(R"({"x":0.1})")), std::string("  x : 0.1\n"));
    });

    IT("writes doubles with a '.0' or an exponent, and integers in plain decimal", {
      ASSERT_EQUAL(
          BodyOf(
              Json(
                  R"({"a":2.0,"b":1e300,"c":-0.0,"d":-9223372036854775808,"e":0.3333333333333333})"
              ),
              Json(
                  R"({"a":2.0,"b":1e300,"c":-0.0,"d":-9223372036854775808,"e":0.3333333333333333})"
              )
          ),
          std::string(
              "  a : 2.0\n  b : 1e+300\n  c : -0.0\n  d : -9223372036854775808\n"
              "  e : 0.3333333333333333\n"
          )
      );
    });

    IT("fails on NaN or infinity, naming the field", {
      core::Value entity = Json(R"({"light":{"intensity":0.0}})");
      std::get<core::OrderedMap<core::Value>>(
          std::get<core::OrderedMap<core::Value>>(entity.data).Get("light").data
      )
          .Insert("intensity", core::Value{std::numeric_limits<double>::infinity()});
      ASSERT_EQUAL(
          Write(entity, Json("{}")),
          std::string(
              "error: Can't write NaN or infinity at light.intensity; Calamus has no "
              "syntax for them"
          )
      );
    });

    IT("fails on NaN in a recorded default", {
      core::Value base = Json(R"({"x":0.0})");
      std::get<core::OrderedMap<core::Value>>(base.data).Insert(
          "x", core::Value{std::numeric_limits<double>::quiet_NaN()}
      );
      ASSERT_EQUAL(
          Write(Json(R"({"x":1.0})"), base),
          std::string(
              "error: Can't write NaN or infinity in the base at x; Calamus has no "
              "syntax for them"
          )
      );
    });
  });

  DESCRIBE("Formatting", {
    IT("quotes keys that aren't identifiers", {
      core::Value entity = Json(R"({"door frame":1,"a.b":2,"_ok1":3,"":4,"9":5})");
      ASSERT_EQUAL(
          BodyOf(entity, entity),
          std::string("  \"door frame\" : 1\n  \"a.b\" : 2\n  _ok1 : 3\n  \"\" : 4\n  \"9\" : 5\n")
      );
    });

    IT("writes strings quoted, with JSON's escapes", {
      core::Value entity = Json(R"({"s":"a \"b\" | c # d\né"})");
      ASSERT_EQUAL(
          BodyOf(entity, entity), std::string("  s : \"a \\\"b\\\" | c # d\\n\xC3\xA9\"\n")
      );
    });

    IT("writes an empty object as a bare block line and an empty array as []", {
      core::Value entity = Json(R"({"light":{},"tags":[]})");
      ASSERT_EQUAL(BodyOf(entity, entity), std::string("  light:\n  tags : []\n"));
    });

    IT("writes arrays without objects inline, nesting them", {
      core::Value entity = Json(R"({"grid":[[1,2],[3],[]],"mixed":[null,true,"s",1.5]})");
      ASSERT_EQUAL(
          BodyOf(entity, entity),
          std::string("  grid : [[1, 2], [3], []]\n  mixed : [null, true, \"s\", 1.5]\n")
      );
    });

    IT("writes any array that holds an object as '-' items", {
      core::Value entity = Json(R"({"list":[1,{"x":2},[3,{"y":4}],[],{}]})");
      ASSERT_EQUAL(
          BodyOf(entity, entity), std::string(
                                      "  list:\n"
                                      "    - 1\n"
                                      "    -\n"
                                      "      x : 2\n"
                                      "    -\n"
                                      "      - 3\n"
                                      "      -\n"
                                      "        y : 4\n"
                                      "    - []\n"
                                      "    -\n"
                                  )
      );
    });
  });

  DESCRIBE("Round trips", {
    IT("loads back what it wrote, with nothing to heal", {
      auto result = RoundTrip(Door, Prefab);
      ASSERT_EQUAL(Canonical(result.value), Canonical(Door));
      ASSERT_FALSE(result.needsRewrite);
      ASSERT_TRUE(result.baseHashMatches);
      ASSERT_EQUAL(result.overrides.size(), std::size_t{5});
      ASSERT_TRUE(result.promoted.empty() && result.reset.empty() && result.moved.empty());
    });

    IT("loads back additions and whole-value changes", {
      core::Value base = Json(R"({"light":null,"tags":["a"],"x":1.0})");
      core::Value entity =
          Json(R"({"light":{"intensity":2.0},"tags":[[1],{"y":[]}],"x":1,"extra":{"z":[]}})");
      auto result = RoundTrip(entity, base);
      ASSERT_EQUAL(Canonical(result.value), Canonical(entity));
      ASSERT_FALSE(result.needsRewrite);
    });

    IT("loads back an entity written over a base that isn't an Object", {
      core::Value entity = Json(R"({"a":{"b":{}},"c":"d"})");
      auto result = RoundTrip(entity, Json("{}"));
      ASSERT_EQUAL(Canonical(result.value), Canonical(entity));
    });

    IT("writes the same text again after a round trip", {
      auto result = RoundTrip(Door, Prefab);
      ASSERT_EQUAL(Write(result.value, Prefab), Write(Door, Prefab));
    });
  });
});
