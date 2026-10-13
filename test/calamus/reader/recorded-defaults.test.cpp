#include <cimmerian/test.hpp>
#include "reader-support.hpp"

#include <string>

using namespace calamus_reader_test;

DESCRIBE("Calamus reader", {
  DESCRIBE("Recorded defaults", {
    IT("must be a valid scalar or inline array", {
      ASSERT_EQUAL(
          BodyErrorOf("  x : 1 | zero\n"),
          std::string("4:11 Strings must be quoted, as in \"zero\"")
      );
      ASSERT_EQUAL(
          BodyErrorOf("  x : 1 |\n"), std::string("4:10 Expected a recorded default after '|'")
      );
    });

    IT("are allowed on unmarked lines and never become part of the body",
       { ASSERT_EQUAL(BodyOf("  x : 1 | 0\n"), std::string(R"({"x":1})")); });

    IT("can only follow a field's value", {
      ASSERT_EQUAL(
          BodyErrorOf("  a:\n    - 1 | 0\n"),
          std::string("5:9 A recorded default can only follow a field's value")
      );
    });
  });
});
