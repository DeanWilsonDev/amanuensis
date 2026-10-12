#include <cimmerian/test.hpp>
#include "reader-support.hpp"

#include <string>

using namespace calamus_reader_test;

DESCRIBE("Calamus reader", {
  DESCRIBE("Blocks", {
    IT("reads a block with nothing under it as an empty Object", {
      ASSERT_EQUAL(BodyOf("  light:\n  x : 1\n"), std::string(R"({"light":{},"x":1})"));
      ASSERT_EQUAL(BodyOf("  light:\n"), std::string(R"({"light":{}})"));
    });

    IT("reads '-' items as an Array", {
      ASSERT_EQUAL(
          BodyOf("  list:\n    - 1\n    - \"two\"\n    - [3, 4]\n    -\n      x : 5\n    -\n"),
          std::string(R"({"list":[1,"two",[3,4],{"x":5},{}]})")
      );
    });

    IT("reads '-' blocks with '-' items under them as nested arrays", {
      ASSERT_EQUAL(
          BodyOf("  grid:\n    -\n      - 1\n      -\n        x : 2\n    -\n      - 3\n"),
          std::string(R"({"grid":[[1,{"x":2}],[3]]})")
      );
    });

    IT("rejects a block that mixes fields and '-' items", {
      ASSERT_EQUAL(
          BodyErrorOf("  a:\n    x : 1\n    - 2\n"),
          std::string("6:5 A block can't mix fields and '-' items")
      );
      ASSERT_EQUAL(
          BodyErrorOf("  a:\n    - 2\n    x : 1\n"),
          std::string("6:5 A block can't mix fields and '-' items")
      );
    });

    IT("rejects '-' items directly under the root", {
      ASSERT_EQUAL(
          BodyErrorOf("  x : 1\n  - 2\n"),
          std::string("5:3 '-' items can only appear inside an array block")
      );
    });

    IT("needs a space after '-'", {
      ASSERT_EQUAL(BodyErrorOf("  a:\n    -1\n"), std::string("5:6 Expected a space after '-'"));
    });
  });
});
