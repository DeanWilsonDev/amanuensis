#include <cimmerian/test.hpp>
#include "writer-support.hpp"

#include <string>

using namespace calamus_writer_test;

DESCRIBE("Calamus writer", {
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
});
