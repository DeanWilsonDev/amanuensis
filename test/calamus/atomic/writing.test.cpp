#include <cimmerian/test.hpp>
#include "atomic-support.hpp"
#include "support/values.hpp"
#include <amanuensis/calamus/writer.hpp>
#include <amanuensis/serialization/serialization.hpp>

#include <string>

using namespace calamus_atomic_test;

DESCRIBE("Atomic objects", {
  DESCRIBE("Writing", {
    IT("marks every component when one differs, each with its recorded default", {
      ASSERT_EQUAL(
          BodyOf(Turned, Upright, Atomic({{"orientation"}})), std::string(
                                                                  "* orientation:\n"
                                                                  "*   x : 0.0 | 0.0\n"
                                                                  "*   y : 0.0 | 0.0\n"
                                                                  "*   z : -0.069125 | 0.0\n"
                                                                  "*   w : 0.997608 | 1.0\n"
                                                              )
      );
    });

    IT("marks only the changed components of an object that isn't atomic", {
      ASSERT_EQUAL(
          BodyOf(Turned, Upright), std::string(
                                       "* orientation:\n"
                                       "    x : 0.0\n"
                                       "    y : 0.0\n"
                                       "*   z : -0.069125 | 0.0\n"
                                       "*   w : 0.997608 | 1.0\n"
                                   )
      );
    });

    IT("leaves an unchanged atomic object unmarked", {
      ASSERT_EQUAL(
          BodyOf(Upright, Upright, Atomic({{"orientation"}})),
          std::string("  orientation:\n    x : 0.0\n    y : 0.0\n    z : 0.0\n    w : 1.0\n")
      );
    });

    IT("marks every line at any depth inside an atomic object", {
      core::Value base = Json(R"({"light":{"intensity":1.0,"colour":{"r":1.0,"g":1.0}}})");
      core::Value entity = Json(R"({"light":{"intensity":1.0,"colour":{"r":0.5,"g":1.0}}})");
      ASSERT_EQUAL(
          BodyOf(entity, base, Atomic({{"light"}})), std::string(
                                                         "* light:\n"
                                                         "*   intensity : 1.0 | 1.0\n"
                                                         "*   colour:\n"
                                                         "*     r : 0.5 | 1.0\n"
                                                         "*     g : 1.0 | 1.0\n"
                                                     )
      );
    });

    IT("ignores a change that only removes a key the base has", {
      core::Value base = Json(R"({"colour":{"r":1.0,"g":1.0,"b":1.0}})");
      core::Value entity = Json(R"({"colour":{"r":1.0,"g":1.0}})");
      ASSERT_EQUAL(
          BodyOf(entity, base, Atomic({{"colour"}})),
          std::string("  colour:\n    r : 1.0\n    g : 1.0\n")
      );
    });

    IT("ignores paths where the entity has no Object", {
      core::Value entity = Json(R"({"tags":["a"],"x":1})");
      ASSERT_EQUAL(
          BodyOf(entity, Json(R"({"tags":["b"],"x":1})"), Atomic({{"tags"}, {"x"}, {"missing"}})),
          std::string("* tags : [\"a\"] | [\"b\"]\n  x : 1\n")
      );
    });

    IT("works from the C++ types end to end", {
      Door prefab;
      Door door;
      door.orientation.z = -0.069125;
      door.light.colour.g = 0.5;
      calamus::WriterOptions options;
      options.atomic = amanuensis::AtomicPaths(door);
      std::string body = BodyOf(amanuensis::ToValue(door), amanuensis::ToValue(prefab), options);
      ASSERT_TRUE(
          body.find("*   x : 0.0 | 0.0\n*   y : 0.0 | 0.0\n*   z : -0.069125 | 0.0\n") !=
          std::string::npos
      );
      ASSERT_TRUE(
          body.find("*   tint:\n*     r : 1.0 | 1.0\n*     g : 0.5 | 1.0\n") != std::string::npos
      );
      ASSERT_TRUE(body.find("  position:\n    x : 0.0\n") != std::string::npos);
    });
  });
});
