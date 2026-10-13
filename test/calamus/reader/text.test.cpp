#include <cimmerian/test.hpp>
#include "reader-support.hpp"

#include <string>

using namespace calamus_reader_test;

DESCRIBE("Calamus reader", {
  DESCRIBE("Text", {
    IT("ignores blank lines anywhere and trailing whitespace", {
      ASSERT_EQUAL(
          BodyOf("  a:\n\n    x : 1   \n \t \n    y : \"s\"\t\n\n  b : 2\n"),
          std::string(R"({"a":{"x":1,"y":"s"},"b":2})")
      );
    });

    IT("accepts CRLF line endings", {
      auto result = calamus::Reader::ParseString("@version 1\r\n@type Door\r\n\r\n  x : 1\r\n");
      REQUIRE_TRUE(result.succeeded);
      ASSERT_EQUAL(Minified(result.document.body), std::string(R"({"x":1})"));
    });

    IT("rejects a carriage return without a line feed", {
      ASSERT_EQUAL(
          BodyErrorOf("  x : 1\r  y : 2\n"),
          std::string("4:8 A carriage return must be followed by a line feed")
      );
      ASSERT_EQUAL(
          BodyErrorOf("  x : 1\r"),
          std::string("4:8 A carriage return must be followed by a line feed")
      );
    });

    IT("skips a leading byte-order mark", {
      auto result = calamus::Reader::ParseString("\xEF\xBB\xBF@version 1\n@type Door\n");
      ASSERT_TRUE(result.succeeded);
    });

    IT("doesn't need a final newline",
       { ASSERT_EQUAL(BodyOf("  x : 1"), std::string(R"({"x":1})")); });
  });
});
