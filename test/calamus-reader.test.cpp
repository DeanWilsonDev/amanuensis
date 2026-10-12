#include <cimmerian/test.hpp>
#include <amanuensis/calamus/reader.hpp>
#include <amanuensis/json/writer.hpp>

#include <string>
#include <string_view>

namespace calamus = amanuensis::calamus;
namespace core = amanuensis::core;

static const std::string Header = "@version 1\n@type Door\n\n";

static std::string Minified(const core::Value& value)
{
  amanuensis::json::WriterOptions options;
  options.pretty = false;
  options.trailingNewline = false;
  return amanuensis::json::Writer::WriteToString(value, options);
}

// The body as minified JSON, or the error.
static std::string BodyOf(std::string_view body)
{
  auto result = calamus::Reader::ParseString(Header + std::string(body));
  if (!result.succeeded) {
    return "error: " + result.error.message;
  }
  return Minified(result.document.body);
}

// "line:column message", or "ok" when the text reads.
static std::string ErrorOf(std::string_view text)
{
  auto result = calamus::Reader::ParseString(text);
  if (result.succeeded) {
    return "ok";
  }
  return std::to_string(result.error.line) + ":" + std::to_string(result.error.column) + " " +
         result.error.message;
}

static std::string BodyErrorOf(std::string_view body)
{
  return ErrorOf(Header + std::string(body));
}

// Each field as its dotted path, with a '*' when it's marked.
static std::string FieldsOf(std::string_view body)
{
  auto result = calamus::Reader::ParseString(Header + std::string(body));
  if (!result.succeeded) {
    return "error: " + result.error.message;
  }
  std::string fields;
  for (const calamus::Field& field : result.document.fields) {
    if (!fields.empty()) {
      fields += " ";
    }
    for (std::size_t index = 0; index < field.path.size(); ++index) {
      fields += (index == 0 ? "" : ".") + field.path[index];
    }
    if (field.marked) {
      fields += "*";
    }
  }
  return fields;
}

static const char* SpecExample = R"calamus(@version 3
@type Door
@id 3f9c2a7be1d04c58
@base "prefabs/wooden_door"
@base_hash 9f2c41e07b3d5a18

* position:
*   x : 46.103531 | 0.0
*   y : 108.07968 | 0.0
    z : 0.0
  scale:
    x : 1.0
    y : 1.0
    z : 1.0
* orientation:
*   x : 0.0 | 0.0
*   y : 0.0 | 0.0
*   z : -0.069125 | 0.0
*   w : 0.997608 | 1.0
  name : ""
  flags : 160
* locked : true | false
  tags : ["puzzle", "act1"]
  waypoints:
    -
      x : 0.0
      z : 2.5
    -
      x : 4.0
      z : 2.5
* light:
*   intensity : 2.5 | 1.0
    colour:
      r : 1.0
      g : 0.9
      b : 0.8
)calamus";

DESCRIBE("Calamus reader", {
  DESCRIBE("The spec's example", {
    IT("reads the header", {
      auto result = calamus::Reader::ParseString(SpecExample);
      REQUIRE_TRUE(result.succeeded);
      const calamus::Header& header = result.document.header;
      ASSERT_EQUAL(header.version, 3);
      ASSERT_EQUAL(header.type, std::string("Door"));
      REQUIRE_TRUE(header.id.has_value());
      ASSERT_EQUAL(*header.id, std::uint64_t{0x3f9c2a7be1d04c58});
      REQUIRE_TRUE(header.base.has_value());
      ASSERT_EQUAL(*header.base, std::string("prefabs/wooden_door"));
      REQUIRE_TRUE(header.baseHash.has_value());
      ASSERT_EQUAL(*header.baseHash, std::uint64_t{0x9f2c41e07b3d5a18});
    });

    IT("reads the body as written", {
      auto result = calamus::Reader::ParseString(SpecExample);
      REQUIRE_TRUE(result.succeeded);
      ASSERT_EQUAL(
          Minified(result.document.body),
          std::string(
              R"({"position":{"x":46.103531,"y":108.07968,"z":0.0},)"
              R"("scale":{"x":1.0,"y":1.0,"z":1.0},)"
              R"("orientation":{"x":0.0,"y":0.0,"z":-0.069125,"w":0.997608},)"
              R"("name":"","flags":160,"locked":true,"tags":["puzzle","act1"],)"
              R"("waypoints":[{"x":0.0,"z":2.5},{"x":4.0,"z":2.5}],)"
              R"("light":{"intensity":2.5,"colour":{"r":1.0,"g":0.9,"b":0.8}}})"
          )
      );
    });

    IT("lists the fields in document order with their marks", {
      auto result = calamus::Reader::ParseString(SpecExample);
      REQUIRE_TRUE(result.succeeded);
      std::string fields;
      for (const calamus::Field& field : result.document.fields) {
        fields += (fields.empty() ? "" : " ") + field.path.back() + (field.marked ? "*" : "");
      }
      ASSERT_EQUAL(
          fields, std::string("x* y* z x y z x* y* z* w* name flags locked* tags waypoints "
                              "intensity* r g b")
      );
    });

    IT("keeps each field's line and recorded default", {
      auto result = calamus::Reader::ParseString(SpecExample);
      REQUIRE_TRUE(result.succeeded);
      const calamus::Field& positionX = result.document.fields[0];
      ASSERT_EQUAL(positionX.line, 8);
      REQUIRE_TRUE(positionX.recordedDefault.has_value());
      ASSERT_EQUAL(Minified(*positionX.recordedDefault), std::string("0.0"));
      ASSERT_FALSE(result.document.fields[2].recordedDefault.has_value());
    });
  });

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
          ErrorOf("@type Door\n\n  x : 1\n"),
          std::string("3:1 Missing the @version header line")
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
      ASSERT_EQUAL(ErrorOf("@version 1\n@id xyz\n"), std::string("2:5 @id must be 1 to 16 hex digits"));
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

    IT("doesn't need a final newline", {
      ASSERT_EQUAL(BodyOf("  x : 1"), std::string(R"({"x":1})"));
    });
  });

  DESCRIBE("Marker column and indentation", {
    IT("rejects anything but '*' or a space in the marker column", {
      ASSERT_EQUAL(
          BodyErrorOf("position:\n"),
          std::string("4:1 Expected '*' or a space in the marker column")
      );
    });

    IT("needs a space after the marker", {
      ASSERT_EQUAL(
          BodyErrorOf("*position:\n"), std::string("4:2 Expected a space after the marker column")
      );
      ASSERT_EQUAL(BodyErrorOf("*\n"), std::string("4:2 Expected a field after the marker"));
    });

    IT("rejects tabs in indentation", {
      ASSERT_EQUAL(BodyErrorOf("\tx : 1\n"), std::string("4:1 Tabs aren't allowed in indentation"));
      ASSERT_EQUAL(
          BodyErrorOf("  a:\n  \tx : 1\n"), std::string("5:3 Tabs aren't allowed in indentation")
      );
    });

    IT("rejects an odd number of spaces", {
      ASSERT_EQUAL(
          BodyErrorOf("  a:\n     x : 1\n"),
          std::string("5:6 Indentation must be a multiple of two spaces")
      );
    });

    IT("rejects a line more than one level deeper than its parent", {
      ASSERT_EQUAL(
          BodyErrorOf("  a:\n      x : 1\n"),
          std::string("5:7 Indented more than one level under the line above")
      );
    });

    IT("rejects lines indented under a line that isn't a block", {
      ASSERT_EQUAL(
          BodyErrorOf("  a : 1\n    x : 1\n"),
          std::string("5:5 Only a 'key:' or '-' block line can have lines indented under it")
      );
      ASSERT_EQUAL(
          BodyErrorOf("  a:\n    - 1\n      x : 1\n"),
          std::string("6:7 Only a 'key:' or '-' block line can have lines indented under it")
      );
    });

    IT("rejects an indented first body line", {
      ASSERT_EQUAL(
          BodyErrorOf("    x : 1\n"), std::string("4:5 The first line of the body can't be indented")
      );
    });

    IT("closes a block when the indentation drops back", {
      ASSERT_EQUAL(
          BodyOf("  a:\n    b:\n      c : 1\n    d : 2\n  e : 3\n"),
          std::string(R"({"a":{"b":{"c":1},"d":2},"e":3})")
      );
    });
  });

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

  DESCRIBE("Values", {
    IT("reads scalars, with a '.' or exponent making a Double", {
      ASSERT_EQUAL(
          BodyOf("  a : null\n  b : true\n  c : false\n  d : -12\n  e : 1.0\n  f : 2e3\n  g : \"\"\n"),
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
      ASSERT_EQUAL(BodyOf("  x:1|0\n  y   :   2   |   0\n  z:\n"), std::string(R"({"x":1,"y":2,"z":{}})"));
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
      ASSERT_EQUAL(
          BodyErrorOf("  a : 1\n  \"a\":\n"), std::string("5:3 Duplicate key 'a'")
      );
    });

    IT("reports string errors at their line and column", {
      ASSERT_EQUAL(
          BodyErrorOf("  a : 1\n  b : \"x\\q\"\n"), std::string("5:11 Invalid escape sequence '\\q'")
      );
    });
  });

  DESCRIBE("Comments", {
    IT("rejects '#' outside a string", {
      ASSERT_EQUAL(
          BodyErrorOf("  x : 1  # note\n"),
          std::string("4:10 Comments aren't supported in Calamus v1")
      );
      ASSERT_EQUAL(
          BodyErrorOf("  # note\n"), std::string("4:3 Comments aren't supported in Calamus v1")
      );
      ASSERT_EQUAL(
          BodyErrorOf("  x : # note\n"), std::string("4:7 Comments aren't supported in Calamus v1")
      );
    });
  });

  DESCRIBE("Recorded defaults", {
    IT("must be a valid scalar or inline array", {
      ASSERT_EQUAL(
          BodyErrorOf("  x : 1 | zero\n"), std::string("4:11 Strings must be quoted, as in \"zero\"")
      );
      ASSERT_EQUAL(
          BodyErrorOf("  x : 1 |\n"), std::string("4:10 Expected a recorded default after '|'")
      );
    });

    IT("are allowed on unmarked lines and never become part of the body", {
      ASSERT_EQUAL(BodyOf("  x : 1 | 0\n"), std::string(R"({"x":1})"));
    });

    IT("can only follow a field's value", {
      ASSERT_EQUAL(
          BodyErrorOf("  a:\n    - 1 | 0\n"),
          std::string("5:9 A recorded default can only follow a field's value")
      );
    });
  });

  DESCRIBE("Fields", {
    IT("honours marks on field lines", {
      ASSERT_EQUAL(FieldsOf("* x : 1\n  y : 2\n"), std::string("x* y"));
    });

    IT("treats a block line as a field when its block is an array or an empty object", {
      ASSERT_EQUAL(
          FieldsOf("* list:\n    - 1\n* empty:\n  object:\n*   z : 1\n"),
          std::string("list* empty* object.z*")
      );
    });

    IT("ignores the mark on a block line that opens a non-empty object", {
      ASSERT_EQUAL(FieldsOf("* a:\n    x : 1\n"), std::string("a.x"));
    });

    IT("never lists lines inside an array as fields", {
      ASSERT_EQUAL(
          FieldsOf("  list:\n    -\n*     x : 1\n      inner:\n        y : 2\n"),
          std::string("list")
      );
    });

    IT("lists an inline array as one field", {
      ASSERT_EQUAL(FieldsOf("* tags : [\"a\"]\n"), std::string("tags*"));
    });
  });
});
