#include <cimmerian/test.hpp>
#include "reader-support.hpp"

#include <string>

using namespace calamus_reader_test;

DESCRIBE("Calamus reader", {
  DESCRIBE("Values", {
    IT("reads scalars, with a '.' or exponent making a Double", {
      ASSERT_EQUAL(
          BodyOf(
              "  a : null\n  b : true\n  c : false\n  d : -12\n  e : 1.0\n  f : 2e3\n  g : \"\"\n"
          ),
          std::string(R"({"a":null,"b":true,"c":false,"d":-12,"e":1.0,"f":2000.0,"g":""})")
      );
    });

    IT("reads nested and empty inline arrays", {
      ASSERT_EQUAL(
          BodyOf("  a : [[1, 2], [3], []]\n  b : [ ]\n  c : [ 1 ,2 ]\n"),
          std::string(R"({"a":[[1,2],[3],[]],"b":[],"c":[1,2]})")
      );
    });

    IT("treats ',', ':', '|' and '#' inside a string as characters", {
      ASSERT_EQUAL(
          BodyOf("  a : \"x, y: z | w # v\"\n  b : [\"p,q\", \"]\"]\n"),
          std::string(R"({"a":"x, y: z | w # v","b":["p,q","]"]})")
      );
    });

    IT("reads quoted keys", {
      ASSERT_EQUAL(
          BodyOf("  \"door frame\" : 1\n  \"a:b\":\n    \"\" : 2\n"),
          std::string(R"({"door frame":1,"a:b":{"":2}})")
      );
    });

    IT("allows any spacing around ':' and '|'", {
      ASSERT_EQUAL(
          BodyOf("  x:1|0\n  y   :   2   |   0\n  z:\n"), std::string(R"({"x":1,"y":2,"z":{}})")
      );
    });

    IT("rejects an unquoted string", {
      ASSERT_EQUAL(
          BodyErrorOf("  name : door\n"), std::string("4:10 Strings must be quoted, as in \"door\"")
      );
    });

    IT("rejects an inline object", {
      ASSERT_EQUAL(
          BodyErrorOf("  a : {}\n"),
          std::string("4:7 Objects can't be written inline; open a block with 'key:' instead")
      );
    });

    IT("rejects a trailing comma and an array left open at the end of the line", {
      ASSERT_EQUAL(BodyErrorOf("  a : [1, 2, ]\n"), std::string("4:14 Trailing comma in array"));
      ASSERT_EQUAL(
          BodyErrorOf("  a : [1, 2\n"),
          std::string("4:12 Unterminated array; an inline array must end on its own line")
      );
    });

    IT("rejects text after the value", {
      ASSERT_EQUAL(BodyErrorOf("  a : 1 2\n"), std::string("4:9 Unexpected '2' after the value"));
      ASSERT_EQUAL(
          BodyErrorOf("  a : 1 | 0 | 0\n"),
          std::string("4:13 Unexpected '|' after the recorded default")
      );
    });

    IT("rejects a key with no ':'", {
      ASSERT_EQUAL(BodyErrorOf("  a 1\n"), std::string("4:5 Expected ':' after the key"));
      ASSERT_EQUAL(BodyErrorOf("  1 : 1\n"), std::string("4:3 Expected a key or a '-' item"));
    });

    IT("rejects a duplicate key at the second one", {
      ASSERT_EQUAL(
          BodyErrorOf("  a:\n    x : 1\n    x : 2\n"), std::string("6:5 Duplicate key 'x'")
      );
      ASSERT_EQUAL(BodyErrorOf("  a : 1\n  \"a\":\n"), std::string("5:3 Duplicate key 'a'"));
    });

    IT("reports string errors at their line and column", {
      ASSERT_EQUAL(
          BodyErrorOf("  a : 1\n  b : \"x\\q\"\n"),
          std::string("5:11 Invalid escape sequence '\\q'")
      );
    });
  });
});
