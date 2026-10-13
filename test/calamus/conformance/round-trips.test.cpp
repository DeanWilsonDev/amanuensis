#include <cimmerian/test.hpp>
#include "conformance-support.hpp"
#include "random-values.hpp"
#include <amanuensis/core/overlay.hpp>

#include <string>
#include <variant>

using namespace conformance_test;

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
});
