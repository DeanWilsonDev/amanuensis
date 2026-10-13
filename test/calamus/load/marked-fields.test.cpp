#include <cimmerian/test.hpp>
#include "load-support.hpp"

#include <string>

using namespace calamus_load_test;

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
});
