#include <cimmerian/test.hpp>
#include "support/values.hpp"
#include <amanuensis/core/diff.hpp>
#include <amanuensis/core/overlay.hpp>
#include <amanuensis/core/stable-hash.hpp>
#include <amanuensis/core/value.hpp>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

DESCRIBE("Value utilities", {
  DESCRIBE("Overlay", {
    IT("merges objects key by key, keeping base order and appending new keys", {
      core::Value merged = core::Overlay(Json(R"({"a":1,"b":2,"c":3})"), Json(R"({"d":4,"b":20})"));
      ASSERT_EQUAL(Minified(merged), std::string(R"({"a":1,"b":20,"c":3,"d":4})"));
    });

    IT("merges nested objects recursively", {
      core::Value merged = core::Overlay(
          Json(R"({"position":{"x":0.0,"y":0.0,"z":0.0},"name":"door"})"),
          Json(R"({"position":{"x":46.5}})")
      );
      ASSERT_EQUAL(
          Minified(merged), std::string(R"({"position":{"x":46.5,"y":0.0,"z":0.0},"name":"door"})")
      );
    });

    IT("replaces arrays whole", {
      core::Value merged =
          core::Overlay(Json(R"({"tags":["puzzle","act1","door"]})"), Json(R"({"tags":["act2"]})"));
      ASSERT_EQUAL(Minified(merged), std::string(R"({"tags":["act2"]})"));
    });

    IT("replaces a value whose type changes", {
      core::Value merged = core::Overlay(
          Json(R"({"light":{"intensity":1.0},"flags":3})"),
          Json(R"({"light":null,"flags":{"a":true}})")
      );
      ASSERT_EQUAL(Minified(merged), std::string(R"({"light":null,"flags":{"a":true}})"));
    });

    IT("returns the override when either side isn't an object", {
      ASSERT_EQUAL(
          Minified(core::Overlay(Json("1"), Json(R"({"a":1})"))), std::string(R"({"a":1})")
      );
      ASSERT_EQUAL(Minified(core::Overlay(Json(R"({"a":1})"), Json("[1]"))), std::string("[1]"));
    });

    IT("leaves base unchanged", {
      core::Value base = Json(R"({"a":{"b":1}})");
      core::Overlay(base, Json(R"({"a":{"b":2}})"));
      ASSERT_EQUAL(Minified(base), std::string(R"({"a":{"b":1}})"));
    });
  });

  DESCRIBE("Diff", {
    IT("finds nothing between equal values", {
      ASSERT_TRUE(
          core::Diff(Json(R"({"a":[1,{"b":2}],"c":"x"})"), Json(R"({"a":[1,{"b":2}],"c":"x"})"))
              .empty()
      );
    });

    IT("reports a changed field at its full path", {
      auto changes = core::Diff(
          Json(R"({"position":{"x":0.0,"y":0.0},"name":"door"})"),
          Json(R"({"position":{"x":46.5,"y":0.0},"name":"door"})")
      );
      ASSERT_EQUAL(changes, (std::vector<core::KeyPath>{{"position", "x"}}));
    });

    IT("compares arrays whole and reports the array's path", {
      auto changes = core::Diff(
          Json(R"({"waypoints":[{"x":0.0},{"x":4.0}]})"),
          Json(R"({"waypoints":[{"x":0.0},{"x":5.0}]})")
      );
      ASSERT_EQUAL(changes, (std::vector<core::KeyPath>{{"waypoints"}}));
    });

    IT("reports added keys, then removed keys", {
      auto changes =
          core::Diff(Json(R"({"a":1,"gone":2,"b":3})"), Json(R"({"new":0,"a":1,"b":3})"));
      ASSERT_EQUAL(changes, (std::vector<core::KeyPath>{{"new"}, {"gone"}}));
    });

    IT("ignores key order, inside arrays too", {
      ASSERT_TRUE(core::Diff(Json(R"({"a":1,"b":2})"), Json(R"({"b":2,"a":1})")).empty());
      ASSERT_TRUE(core::Diff(Json(R"([{"x":1,"y":2}])"), Json(R"([{"y":2,"x":1}])")).empty());
    });

    IT("treats an Integer and a Double as different", {
      auto changes = core::Diff(Json(R"({"count":1})"), Json(R"({"count":1.0})"));
      ASSERT_EQUAL(changes, (std::vector<core::KeyPath>{{"count"}}));
    });

    IT("treats 0.0 and -0.0 as different", {
      auto changes = core::Diff(Json(R"({"z":0.0})"), Json(R"({"z":-0.0})"));
      ASSERT_EQUAL(changes, (std::vector<core::KeyPath>{{"z"}}));
    });

    IT("keeps keys that aren't identifiers whole", {
      auto changes =
          core::Diff(Json(R"({"door frame":{"a.b":1}})"), Json(R"({"door frame":{"a.b":2}})"));
      ASSERT_EQUAL(changes, (std::vector<core::KeyPath>{{"door frame", "a.b"}}));
    });

    IT("reports a changed root as the empty path", {
      auto changes = core::Diff(Json("1"), Json("2"));
      ASSERT_EQUAL(changes.size(), 1u);
      ASSERT_TRUE(changes[0].empty());
    });

    IT("finds exactly the overridden fields after an overlay, and none after a second one", {
      core::Value base = Json(R"({"position":{"x":0.0,"y":0.0},"tags":["a"]})");
      core::Value overrides = Json(R"({"position":{"x":46.5},"tags":["b"]})");
      core::Value entity = core::Overlay(base, overrides);
      auto changes = core::Diff(base, entity);
      ASSERT_EQUAL(changes, (std::vector<core::KeyPath>{{"position", "x"}, {"tags"}}));
      ASSERT_TRUE(core::Diff(entity, core::Overlay(entity, overrides)).empty());
    });
  });

  DESCRIBE("StableHash", {
    // Golden values from an independent implementation of the encoding in
    // stable-hash.hpp. A change here changes every @base_hash on disk.
    IT("matches the documented encoding for scalars", {
      ASSERT_EQUAL(core::StableHash(Json("null")), std::uint64_t{0xaf63bd4c8601b7dfULL});
      ASSERT_EQUAL(core::StableHash(Json("true")), std::uint64_t{0x082f2307b4e88e77ULL});
      ASSERT_EQUAL(core::StableHash(Json("false")), std::uint64_t{0x082f2207b4e88cc4ULL});
      ASSERT_EQUAL(core::StableHash(Json("1")), std::uint64_t{0xedde65ec42d6cbc4ULL});
      ASSERT_EQUAL(core::StableHash(Json("-1")), std::uint64_t{0xaf94b0dfc57cce5dULL});
      ASSERT_EQUAL(core::StableHash(Json("1.0")), std::uint64_t{0x79384a97b8fca0cbULL});
      ASSERT_EQUAL(core::StableHash(Json("-0.0")), std::uint64_t{0x796e5797b92a4652ULL});
      ASSERT_EQUAL(core::StableHash(Json(R"("a")")), std::uint64_t{0x73d40f607f1b3ba9ULL});
    });

    IT("matches the documented encoding for containers", {
      ASSERT_EQUAL(core::StableHash(Json("[]")), std::uint64_t{0x04f0d7663d895b60ULL});
      ASSERT_EQUAL(core::StableHash(Json("{}")), std::uint64_t{0xbf2fd77efb5a3d99ULL});
      ASSERT_EQUAL(core::StableHash(Json(R"(["ab"])")), std::uint64_t{0xe93f68697f1fa822ULL});
      ASSERT_EQUAL(core::StableHash(Json(R"(["a","b"])")), std::uint64_t{0x0f3560efc6ffe82fULL});
      ASSERT_EQUAL(
          core::StableHash(Json(R"({"b":1,"a":[true,null,"x"]})")),
          std::uint64_t{0x1cd3b9d792133634ULL}
      );
      ASSERT_EQUAL(
          core::StableHash(
              Json(R"({"position":{"x":46.103531,"y":108.07968,"z":0.0},"name":"","locked":true})")
          ),
          std::uint64_t{0x1522583e6ba6ddffULL}
      );
    });

    IT("ignores key order", {
      ASSERT_EQUAL(
          core::StableHash(Json(R"({"a":[true,null,"x"],"b":1})")),
          core::StableHash(Json(R"({"b":1,"a":[true,null,"x"]})"))
      );
    });

    IT("tells apart values that print alike", {
      ASSERT_TRUE(core::StableHash(Json("1")) != core::StableHash(Json("1.0")));
      ASSERT_TRUE(core::StableHash(Json(R"(["ab"])")) != core::StableHash(Json(R"(["a","b"])")));
      ASSERT_TRUE(core::StableHash(Json(R"({"a":"b"})")) != core::StableHash(Json(R"({"ab":""})")));
    });
  });
});
