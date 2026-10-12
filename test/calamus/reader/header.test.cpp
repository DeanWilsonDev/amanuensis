#include <cimmerian/test.hpp>
#include "reader-support.hpp"

#include <cstdint>
#include <string>

using namespace calamus_reader_test;

DESCRIBE("Calamus reader", {
  DESCRIBE("Header", {
    IT("needs only @version and @type", {
      auto result = calamus::Reader::ParseString("@version 1\n@type Door\n");
      REQUIRE_TRUE(result.succeeded);
      ASSERT_FALSE(result.document.header.id.has_value());
      ASSERT_FALSE(result.document.header.base.has_value());
      ASSERT_FALSE(result.document.header.baseHash.has_value());
      ASSERT_EQUAL(Minified(result.document.body), std::string("{}"));
    });

    IT("reads an @id of 1 to 16 hex digits in either case", {
      auto result = calamus::Reader::ParseString("@version 1\n@type Door\n@id 3Fa\n");
      REQUIRE_TRUE(result.succeeded);
      ASSERT_EQUAL(*result.document.header.id, std::uint64_t{0x3fa});
    });

    IT("allows several spaces after the name, and type names with '.' and ':'", {
      auto result = calamus::Reader::ParseString("@version   12\n@type    game::doors.Wooden_2\n");
      REQUIRE_TRUE(result.succeeded);
      ASSERT_EQUAL(result.document.header.version, 12);
      ASSERT_EQUAL(result.document.header.type, std::string("game::doors.Wooden_2"));
    });

    IT("reads @base with escapes", {
      auto result =
          calamus::Reader::ParseString("@version 1\n@type Door\n@base \"prefabs/\\\"odd\\\"\"\n");
      REQUIRE_TRUE(result.succeeded);
      ASSERT_EQUAL(*result.document.header.base, std::string("prefabs/\"odd\""));
    });

    IT("rejects a missing @version or @type where the header ends", {
      ASSERT_EQUAL(
          ErrorOf("@type Door\n\n  x : 1\n"), std::string("3:1 Missing the @version header line")
      );
      ASSERT_EQUAL(ErrorOf("@version 1"), std::string("1:11 Missing the @type header line"));
      ASSERT_EQUAL(ErrorOf(""), std::string("1:1 Missing the @version header line"));
    });

    IT("rejects unknown and repeated header lines", {
      ASSERT_EQUAL(
          ErrorOf("@version 1\n@calamus 2\n"), std::string("2:1 Unknown header line '@calamus'")
      );
      ASSERT_EQUAL(
          ErrorOf("@version 1\n@type Door\n@type Door\n"),
          std::string("3:1 Repeated header line '@type'")
      );
    });

    IT("rejects a header line in the body", {
      ASSERT_EQUAL(
          ErrorOf("@version 1\n@type Door\n  x : 1\n@id 1\n"),
          std::string("4:1 Header lines must come before the body")
      );
    });

    IT("rejects a header line without a value or a space", {
      ASSERT_EQUAL(ErrorOf("@version\n"), std::string("1:9 Expected a value after '@version'"));
      ASSERT_EQUAL(ErrorOf("@version:1\n"), std::string("1:9 Expected a space after '@version'"));
      ASSERT_EQUAL(ErrorOf("@ version 1\n"), std::string("1:2 Expected a header name after '@'"));
    });

    IT("rejects invalid header values at the value", {
      ASSERT_EQUAL(
          ErrorOf("@version 0\n"), std::string("1:10 @version must be a whole number of at least 1")
      );
      ASSERT_EQUAL(
          ErrorOf("@version 1.5\n"),
          std::string("1:10 @version must be a whole number of at least 1")
      );
      ASSERT_EQUAL(
          ErrorOf("@version 1\n@type 2Door\n"),
          std::string(
              "2:7 @type must be a name: a letter or '_', then letters, digits, '_', '.' or ':'"
          )
      );
      ASSERT_EQUAL(
          ErrorOf("@version 1\n@id xyz\n"), std::string("2:5 @id must be 1 to 16 hex digits")
      );
      ASSERT_EQUAL(
          ErrorOf("@version 1\n@id 3f9c2a7be1d04c58a\n"),
          std::string("2:5 @id must be 1 to 16 hex digits")
      );
      ASSERT_EQUAL(
          ErrorOf("@version 1\n@base_hash 9f2c41\n"),
          std::string("2:12 @base_hash must be exactly 16 hex digits")
      );
      ASSERT_EQUAL(
          ErrorOf("@version 1\n@base prefabs/door\n"),
          std::string("2:7 @base must be a quoted string")
      );
      ASSERT_EQUAL(
          ErrorOf("@version 1\n@base \"a\" \"b\"\n"),
          std::string("2:11 Unexpected text after the @base string")
      );
    });
  });
});
