#include <cimmerian/test.hpp>
#include "writer-support.hpp"

#include <cstddef>

using namespace calamus_writer_test;

DESCRIBE("Calamus writer", {
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
