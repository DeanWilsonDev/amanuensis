#include <cimmerian/test.hpp>
#include "load-support.hpp"

#include <string>

using namespace calamus_load_test;

DESCRIBE("Calamus load", {
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
});
