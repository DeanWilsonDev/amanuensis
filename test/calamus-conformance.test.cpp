
#include <cimmerian/test.hpp>
#include "calamus/canonical.hpp"
#include <amanuensis/calamus/reader.hpp>
#include <amanuensis/calamus/writer.hpp>
#include <amanuensis/core/overlay.hpp>
#include <amanuensis/json/reader.hpp>
#include <amanuensis/json/writer.hpp>
#include <amanuensis/serialization/serialization.hpp>

#include <bit>
#include <cmath>
#include <cstdint>
#include <random>
#include <string>
#include <string_view>
#include <vector>

namespace calamus = amanuensis::calamus;
namespace core = amanuensis::core;

using Array = std::vector<core::Value>;
using Object = core::OrderedMap<core::Value>;

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

static std::string Canonical(const core::Value& value)
{
  return Minified(calamus::Canonicalise(value));
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

// A parse error comes back as the loaded value.
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

static std::string Heal(const std::string& text, const core::Value& base)
{
  return Write(Load(text, base).value, base);
}

static std::string Replace(std::string text, std::string_view from, std::string_view to)
{
  std::size_t at = text.find(from);
  if (at != std::string::npos) {
    text.replace(at, from.size(), to);
  }
  return text;
}

static std::string StripMarks(std::string text)
{
  bool lineStart = true;
  for (char& character : text) {
    if (lineStart && character == '*') {
      character = ' ';
    }
    lineStart = character == '\n';
  }
  return text;
}

// Whether every block line with lines under it is marked exactly when one of
// those lines is.
static bool BlockMarksMatch(std::string_view text)
{
  struct Line {
    bool marked;
    std::size_t depth;
    bool opensBlock;
  };
  std::vector<Line> lines;
  std::size_t start = text.find("\n\n");
  while (start != std::string_view::npos && start + 1 < text.size()) {
    std::size_t end = text.find('\n', start + 1);
    std::string_view line = text.substr(start + 1, end - start - 1);
    start = end;
    if (line.size() < 3) {
      continue;
    }
    std::size_t indent = line.find_first_not_of(' ', 2) - 2;
    lines.push_back(Line{line[0] == '*', indent / 2, line.back() == ':' || line.back() == '-'});
  }
  for (std::size_t index = 0; index < lines.size(); ++index) {
    bool anyChild = false;
    bool anyMarkedChild = false;
    for (std::size_t child = index + 1;
         child < lines.size() && lines[child].depth > lines[index].depth; ++child) {
      anyChild = true;
      anyMarkedChild = anyMarkedChild || lines[child].marked;
    }
    if (lines[index].opensBlock && anyChild && lines[index].marked != anyMarkedChild) {
      return false;
    }
  }
  return true;
}

static std::uint64_t Bits(double value)
{
  return std::bit_cast<std::uint64_t>(value);
}

static const core::Value& Get(const core::Value& object, const std::string& key)
{
  return std::get<Object>(object.data).Get(key);
}

static const core::Value Prefab = Json(R"({
  "position": {"x": 0.0, "y": 0.0, "z": 0.0},
  "name": "door",
  "locked": false,
  "tags": ["puzzle"],
  "light": {"intensity": 1.0}
})");

static const core::Value Door = Json(R"({
  "position": {"x": 46.5, "y": 0.0, "z": 0.0},
  "name": "door",
  "locked": true,
  "tags": ["puzzle"],
  "light": {"intensity": 1.0}
})");

static const core::Value EditedPrefab = Json(R"({
  "position": {"x": 10.0, "y": 0.0, "z": 0.0},
  "name": "wooden door",
  "locked": false,
  "tags": ["puzzle"],
  "light": {"intensity": 1.0}
})");

// std::mt19937_64's output is fixed by the standard; the distributions aren't,
// so they're done by hand.
class RandomValues {
public:
  explicit RandomValues(std::uint64_t seed)
      : engine_(seed)
  {
  }

  std::uint64_t Below(std::uint64_t limit) { return engine_() % limit; }

  core::Value Scalar()
  {
    switch (Below(9)) {
    case 0:
      return core::Value();
    case 1:
      return core::Value{Below(2) == 0};
    case 2:
      return core::Value{static_cast<long long>(Below(2001)) - 1000};
    case 3:
      return core::Value{static_cast<long long>(engine_())};
    case 4:
      return core::Value{static_cast<double>(static_cast<long long>(Below(4001)) - 2000) / 8.0};
    case 5: {
      double value = std::bit_cast<double>(engine_());
      return core::Value{std::isfinite(value) ? value : 0.5};
    }
    case 6: {
      float value = std::bit_cast<float>(static_cast<std::uint32_t>(engine_()));
      return core::Value{std::isfinite(value) ? static_cast<double>(value) : 0.25};
    }
    default:
      return core::Value{String()};
    }
  }

  std::string String()
  {
    static const char* pieces[] = {
        "a", "Z", " ", ",", ":", "|", "#", "\"",  "\\", "\n", "\t", "\xC3\xA9", "\xF0\x9F\x98\x80",
        "[", "]", "-", "*", "@", "{", "/", "\x01"
    };
    std::string text;
    for (std::uint64_t count = Below(6); count > 0; --count) {
      text += pieces[Below(std::size(pieces))];
    }
    return text;
  }

  std::string Key()
  {
    static const char* keys[] = {"x",  "y",   "name",     "door frame", "a:b", "",
                                 "#k", "|",   "\xC3\xA9", "_k1",        "9",   "-",
                                 "*",  "@id", "a.b",      "light",      "tags"};
    return keys[Below(std::size(keys))];
  }

  core::Value Any(int depth)
  {
    std::uint64_t kind = depth <= 0 ? 0 : Below(4);
    if (kind == 2) {
      return ArrayOf(depth - 1);
    }
    if (kind == 3) {
      return ObjectOf(depth - 1);
    }
    return Scalar();
  }

  core::Value ArrayOf(int depth)
  {
    Array elements;
    for (std::uint64_t count = Below(4); count > 0; --count) {
      elements.push_back(Any(depth));
    }
    return core::Value{std::move(elements)};
  }

  core::Value ObjectOf(int depth)
  {
    Object object;
    for (std::uint64_t count = Below(5); count > 0; --count) {
      object.Insert(Key(), Any(depth));
    }
    return core::Value{std::move(object)};
  }

  core::Value Mutate(const core::Value& base, int depth)
  {
    const auto* object = std::get_if<Object>(&base.data);
    if (object == nullptr) {
      return Below(3) == 0 ? Any(depth) : base;
    }
    Object mutated;
    for (const auto& [key, value] : object->GetEntries()) {
      std::uint64_t choice = Below(8);
      if (choice == 0) {
        continue;
      }
      if (choice == 1) {
        mutated.Insert(key, Any(depth));
      }
      else if (choice <= 3) {
        mutated.Insert(key, Mutate(value, depth - 1));
      }
      else {
        mutated.Insert(key, value);
      }
    }
    if (Below(3) == 0) {
      mutated.Insert(Key(), Any(depth));
    }
    return core::Value{std::move(mutated)};
  }

private:
  std::mt19937_64 engine_;
};

DESCRIBE("Calamus conformance", {
  DESCRIBE("Round trips", {
    IT("write, read, write gives byte-identical text", {
      std::string written = Write(Door, Prefab);
      ASSERT_EQUAL(Heal(written, Prefab), written);
    });

    IT("holds for random entities over random bases", {
      RandomValues random(20261012);
      for (int iteration = 0; iteration < 400; ++iteration) {
        core::Value base = random.Below(10) == 0 ? random.Any(3) : random.ObjectOf(4);
        core::Value entity = random.Mutate(random.ObjectOf(4), 3);
        if (random.Below(2) == 0) {
          entity = random.Mutate(base, 3);
        }
        if (!std::holds_alternative<Object>(entity.data)) {
          continue;
        }

        std::string written = Write(entity, base);
        calamus::LoadResult loaded = Load(written, base);
        // Keys the entity dropped come back from the base, and an empty
        // object over a base object is no change.
        core::Value expected = core::Overlay(base, entity);
        ASSERT_EQUAL(Canonical(loaded.value), Canonical(expected));
        ASSERT_TRUE(loaded.baseHashMatches);
        ASSERT_FALSE(loaded.needsRewrite);
        ASSERT_TRUE(loaded.refreshed.empty() && loaded.stale.empty() && loaded.moved.empty());

        ASSERT_TRUE(BlockMarksMatch(written));

        std::string regenerated = Write(loaded.value, base);
        ASSERT_EQUAL(Heal(regenerated, base), regenerated);
        if (Canonical(expected) == Canonical(entity)) {
          ASSERT_EQUAL(regenerated, written);
        }

        calamus::LoadResult unmarked = Load(StripMarks(regenerated), base);
        ASSERT_EQUAL(Canonical(unmarked.value), Canonical(loaded.value));
        ASSERT_EQUAL(unmarked.promoted.size(), loaded.overrides.size());
        ASSERT_EQUAL(Write(unmarked.value, base), regenerated);
      }
    });
  });

  DESCRIBE("Hole 1: silent hand edits", {
    IT("promotes a value changed without its '*', and healing adds the mark", {
      std::string edited = Replace(Write(Door, Prefab), "  name : \"door\"", "  name : \"gate\"");
      calamus::LoadResult loaded = Load(edited, Prefab);
      ASSERT_TRUE(loaded.needsRewrite);
      ASSERT_EQUAL(Minified(Get(loaded.value, "name")), std::string("\"gate\""));
      ASSERT_TRUE(
          Heal(edited, Prefab).find("\n* name : \"gate\" | \"door\"\n") != std::string::npos
      );
    });

    IT("follows the new prefab when the prefab changed under the edit", {
      std::string edited = Replace(Write(Door, Prefab), "  name : \"door\"", "  name : \"gate\"");
      calamus::LoadResult loaded = Load(edited, EditedPrefab);
      ASSERT_FALSE(loaded.needsRewrite);
      ASSERT_EQUAL(Minified(Get(loaded.value, "name")), std::string("\"wooden door\""));
    });

    IT("keeps an override whose '*' was deleted", {
      std::string edited = Replace(Write(Door, Prefab), "* locked", "  locked");
      calamus::LoadResult loaded = Load(edited, Prefab);
      ASSERT_EQUAL(Minified(Get(loaded.value, "locked")), std::string("true"));
      ASSERT_EQUAL(Heal(edited, Prefab), Write(Door, Prefab));
    });

    IT("resets a field when its recorded default is copied over the value", {
      std::string edited =
          Replace(Write(Door, Prefab), "* locked : true | false", "* locked : false | false");
      calamus::LoadResult loaded = Load(edited, Prefab);
      ASSERT_TRUE(loaded.needsRewrite);
      ASSERT_TRUE(Heal(edited, Prefab).find("\n  locked : false\n") != std::string::npos);
    });

    IT("resets a field when its line is deleted", {
      std::string edited = Replace(Write(Door, Prefab), "* locked : true | false\n", "");
      calamus::LoadResult loaded = Load(edited, Prefab);
      ASSERT_EQUAL(Minified(Get(loaded.value, "locked")), std::string("false"));
      ASSERT_TRUE(Heal(edited, Prefab).find("\n  locked : false\n") != std::string::npos);
    });

    IT("ignores a deleted recorded default", {
      std::string written = Write(Door, Prefab);
      std::string edited = Replace(written, "* locked : true | false", "* locked : true");
      calamus::LoadResult loaded = Load(edited, Prefab);
      ASSERT_FALSE(loaded.needsRewrite);
      ASSERT_EQUAL(Write(loaded.value, Prefab), written);
    });

    IT("lists overrides the prefab moved under, and the next save records the new default", {
      std::string written = Write(Door, Prefab);
      calamus::LoadResult loaded = Load(written, EditedPrefab);
      ASSERT_EQUAL(loaded.moved.size(), std::size_t{1});
      ASSERT_EQUAL(loaded.moved[0], core::KeyPath({"position", "x"}));
      ASSERT_TRUE(
          Write(loaded.value, EditedPrefab).find("*   x : 46.5 | 10.0\n") != std::string::npos
      );
    });

    IT("keeps the values of a hand-written file with no @base_hash", {
      std::string handWritten = "@version 1\n@type Door\n\n  name : \"gate\"\n  locked : false\n";
      calamus::LoadResult loaded = Load(handWritten, Prefab);
      ASSERT_EQUAL(Minified(Get(loaded.value, "name")), std::string("\"gate\""));
      ASSERT_TRUE(loaded.needsRewrite);
      std::string healed = Heal(handWritten, Prefab);
      ASSERT_TRUE(healed.find("@base_hash ") != std::string::npos);
      ASSERT_TRUE(healed.find("\n* name : \"gate\" | \"door\"\n") != std::string::npos);
    });
  });

  DESCRIBE("Hole 2: a one-item list looks like a scalar", {
    IT("always brackets arrays", {
      core::Value entity = Json(R"({"tags":["a"],"name":"a"})");
      std::string written = Write(entity, Json("{}"));
      ASSERT_TRUE(written.find("* tags : [\"a\"]\n") != std::string::npos);
      ASSERT_EQUAL(Minified(Load(written, Json("{}")).value), Minified(entity));
    });
  });

  DESCRIBE("Hole 3: Integer and Double drift", {
    IT("keeps a whole Double a Double and an Integer an Integer", {
      core::Value entity = Json(R"({"d":1.0,"i":1,"big":1e21})");
      std::string written = Write(entity, Json("{}"));
      ASSERT_TRUE(written.find("* d : 1.0\n* i : 1\n* big : 1e+21\n") != std::string::npos);
      ASSERT_EQUAL(Minified(Load(written, Json("{}")).value), Minified(entity));
    });

    IT("loads a hand-typed Integer into a float field", {
      calamus::LoadResult loaded = Load(Replace(Write(Door, Prefab), "46.5", "46"), Prefab);
      ASSERT_EQUAL(amanuensis::FromValue<float>(Get(Get(loaded.value, "position"), "x")), 46.0f);
    });
  });

  DESCRIBE("Hole 4: float noise", {
    IT("writes a float's shortest text and reads it back as the same float", {
      for (float value : {0.1f, 46.103531f, 108.07968f, -0.069125f, 0.997608f, 1e-38f, 3.4e38f}) {
        core::Value entity{Object{}};
        std::get<Object>(entity.data).Insert("x", core::Value{static_cast<double>(value)});
        std::string written = Write(entity, Json(R"({"x":0.0})"));
        calamus::LoadResult loaded = Load(written, Json(R"({"x":0.0})"));
        ASSERT_EQUAL(amanuensis::FromValue<float>(Get(loaded.value, "x")), value);
        ASSERT_TRUE(written.find("0000") == std::string::npos);
      }
      ASSERT_TRUE(Write(Json(R"({"x":0.0})"), Json("{}")).find("* x : 0.0\n") != std::string::npos);
    });

    IT("reads every double back bit-identical", {
      for (double value : {0.1, 1.0 / 3.0, 5e-324, 1.7976931348623157e308, -0.0, 2.5e-310}) {
        core::Value entity{Object{}};
        std::get<Object>(entity.data).Insert("x", core::Value{value});
        calamus::LoadResult loaded = Load(Write(entity, Json("{}")), Json("{}"));
        ASSERT_EQUAL(Bits(std::get<double>(Get(loaded.value, "x").data)), Bits(value));
      }
    });

    IT("doesn't mark or promote a float field equal to its base", {
      core::Value base{Object{}};
      std::get<Object>(base.data).Insert("x", core::Value{static_cast<double>(0.1f)});
      std::string written = Write(base, base);
      ASSERT_TRUE(written.find("\n  x : 0.1\n") != std::string::npos);
      ASSERT_FALSE(Load(written, base).needsRewrite);
    });
  });

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

  DESCRIBE("Hole 6: keys that aren't identifiers", {
    IT("quotes them and reads them back", {
      core::Value entity = Json(R"({"door frame":1,"a:b":2,"x|y":3,"#":4,"\"q\"":5,"":6,"é":7})");
      std::string written = Write(entity, Json("{}"));
      ASSERT_EQUAL(Minified(Load(written, Json("{}")).value), Minified(entity));
      ASSERT_EQUAL(Heal(written, Json("{}")), written);
    });
  });

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

  DESCRIBE("Hole 9: empty objects", {
    IT("writes and reads an empty object as a bare block line", {
      core::Value entity = Json(R"({"light":{},"tags":[],"nested":{"inner":{}}})");
      std::string written = Write(entity, Json("{}"));
      ASSERT_TRUE(
          written.find("* light:\n* tags : []\n* nested:\n*   inner:\n") != std::string::npos
      );
      ASSERT_EQUAL(Minified(Load(written, Json("{}")).value), Minified(entity));
    });
  });

  DESCRIBE("Hole 10: comments don't survive a save", {
    IT("rejects a comment rather than dropping it", {
      std::string edited =
          Replace(Write(Door, Prefab), "  name : \"door\"", "  name : \"door\"  # front");
      auto read = calamus::Reader::ParseString(edited);
      ASSERT_FALSE(read.succeeded);
      ASSERT_EQUAL(read.error.message, std::string("Comments aren't supported in Calamus v1"));
    });
  });

  DESCRIBE("Hole 11: small things", {
    IT("rejects duplicate keys", {
      ASSERT_FALSE(
          calamus::Reader::ParseString("@version 1\n@type Door\n  x : 1\n  x : 2\n").succeeded
      );
    });

    IT("reads CRLF and trailing whitespace the same as clean LF", {
      std::string written = Write(Door, Prefab);
      std::string messy;
      for (char character : written) {
        if (character == '\n') {
          messy += " \t\r\n";
        }
        else {
          messy += character;
        }
      }
      calamus::LoadResult loaded = Load(messy, Prefab);
      ASSERT_FALSE(loaded.needsRewrite);
      ASSERT_EQUAL(Write(loaded.value, Prefab), written);
    });

    IT("writes LF only and exactly one final newline", {
      std::string written = Write(Door, Prefab);
      ASSERT_TRUE(written.find('\r') == std::string::npos);
      ASSERT_TRUE(written.ends_with("\n") && !written.ends_with("\n\n"));
    });
  });
});
