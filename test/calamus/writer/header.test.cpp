#include <cimmerian/test.hpp>
#include "writer-support.hpp"

#include <string>

using namespace calamus_writer_test;

DESCRIBE("Calamus writer", {
  DESCRIBE("Header", {
    IT("always writes @base_hash, and leaves out @id and @base when there are none", {
      ASSERT_EQUAL(
          Write(Json(R"({"x":1})"), Json(R"({"x":1})")),
          "@version 1\n@type Door\n@base_hash " + Hash(Json(R"({"x":1})")) + "\n\n  x : 1\n"
      );
    });

    IT("ignores the header's own @base_hash", {
      calamus::Header header = DoorHeader();
      header.baseHash = 0x0123456789abcdef;
      auto result = calamus::Writer::WriteToString(header, Json("{}"), Json("{}"));
      ASSERT_EQUAL(result.text, "@version 1\n@type Door\n@base_hash " + Hash(Json("{}")) + "\n");
    });

    IT("writes @id as 16 lowercase hex digits", {
      calamus::Header header = DoorHeader();
      header.id = 0x3FA;
      auto result = calamus::Writer::WriteToString(header, Json("{}"), Json("{}"));
      ASSERT_TRUE(result.text.find("\n@id 00000000000003fa\n") != std::string::npos);
    });

    IT("writes no blank line and one final newline for an empty body", {
      std::string text = Write(Json("{}"), Json("{}"));
      ASSERT_EQUAL(text.find("\n\n"), std::string::npos);
      ASSERT_EQUAL(text.back(), '\n');
    });

    IT("escapes @base", {
      calamus::Header header = DoorHeader();
      header.base = "prefabs/\"odd\"";
      auto result = calamus::Writer::WriteToString(header, Json("{}"), Json("{}"));
      ASSERT_TRUE(result.text.find("\n@base \"prefabs/\\\"odd\\\"\"\n") != std::string::npos);
    });

    IT("refuses a header the reader would reject", {
      calamus::Header header = DoorHeader();
      header.version = 0;
      auto noVersion = calamus::Writer::WriteToString(header, Json("{}"), Json("{}"));
      ASSERT_FALSE(noVersion.succeeded);
      ASSERT_EQUAL(noVersion.error, std::string("@version must be at least 1"));

      header = DoorHeader();
      header.type = "wooden door";
      auto badType = calamus::Writer::WriteToString(header, Json("{}"), Json("{}"));
      ASSERT_FALSE(badType.succeeded);
    });

    IT("refuses an entity that isn't an Object", {
      ASSERT_EQUAL(
          Write(Json("[1]"), Json("{}")), std::string("error: The entity must be an Object")
      );
    });
  });
});
