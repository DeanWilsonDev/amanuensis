#include "cimmerian/test.hpp"
#include <amanuensis/json/reader.hpp>
#include <amanuensis/json/writer.hpp>
#include <amanuensis/core/parse-result.hpp>
#include <amanuensis/core/value.hpp>

#include <string>

// -----------------------------------------------------------------------
// Round-trip — parse → write → parse produces same structure
// -----------------------------------------------------------------------

DESCRIBE("Round-trip", {
  DESCRIBE("Minified round-trip", {
    IT("round-trips a flat object", {
      std::string original_json = R"({"name":"test","count":42,"flag":true})";
      auto first_parse = amanuensis::json::Reader::ParseString(original_json);
      REQUIRE_TRUE(first_parse.succeeded);

      amanuensis::json::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string rewritten = amanuensis::json::Writer::WriteToString(first_parse.value, minified_options);

      auto second_parse = amanuensis::json::Reader::ParseString(rewritten);
      ASSERT_TRUE(second_parse.succeeded);
      ASSERT_EQUAL(second_parse.value.Get("name").AsString(), std::string("test"));
      ASSERT_EQUAL(second_parse.value.Get("count").AsInteger(), 42LL);
      ASSERT_TRUE(second_parse.value.Get("flag").AsBoolean());
    });

    IT("round-trips nested objects and arrays", {
      std::string original_json = R"({"name":"test","values":[1,2,3],"nested":{"flag":true}})";
      auto first_parse = amanuensis::json::Reader::ParseString(original_json);
      REQUIRE_TRUE(first_parse.succeeded);

      amanuensis::json::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string rewritten = amanuensis::json::Writer::WriteToString(first_parse.value, minified_options);

      auto second_parse = amanuensis::json::Reader::ParseString(rewritten);
      ASSERT_TRUE(second_parse.succeeded);
      ASSERT_EQUAL(second_parse.value.Get("name").AsString(), std::string("test"));
      ASSERT_EQUAL(second_parse.value.Get("values").Size(), 3u);
      ASSERT_TRUE(second_parse.value.Get("nested").Get("flag").AsBoolean());
    });

    IT("round-trips strings with escape sequences", {
      std::string original_json = R"({"text":"line1\nline2\ttab\\slash\"quote"})";
      auto first_parse = amanuensis::json::Reader::ParseString(original_json);
      REQUIRE_TRUE(first_parse.succeeded);

      amanuensis::json::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string rewritten = amanuensis::json::Writer::WriteToString(first_parse.value, minified_options);

      auto second_parse = amanuensis::json::Reader::ParseString(rewritten);
      ASSERT_TRUE(second_parse.succeeded);
      ASSERT_EQUAL(
          second_parse.value.Get("text").AsString(), first_parse.value.Get("text").AsString()
      );
    });
  });

  DESCRIBE("Pretty round-trip", {
    IT("pretty-printed output parses back to the same structure", {
      std::string original_json = R"({"a":1,"b":[2,3],"c":{"d":true}})";
      auto first_parse = amanuensis::json::Reader::ParseString(original_json);
      REQUIRE_TRUE(first_parse.succeeded);

      std::string pretty_output = amanuensis::json::Writer::WriteToString(first_parse.value);

      auto second_parse = amanuensis::json::Reader::ParseString(pretty_output);
      ASSERT_TRUE(second_parse.succeeded);
      ASSERT_EQUAL(second_parse.value.Get("a").AsInteger(), 1LL);
      ASSERT_EQUAL(second_parse.value.Get("b").Size(), 2u);
      ASSERT_TRUE(second_parse.value.Get("c").Get("d").AsBoolean());
    });

    IT("second pretty write is byte-identical to first", {
      std::string original_json = R"({"a":1,"b":[2,3],"c":{"d":"hello"}})";
      auto first_parse = amanuensis::json::Reader::ParseString(original_json);
      REQUIRE_TRUE(first_parse.succeeded);

      std::string first_write = amanuensis::json::Writer::WriteToString(first_parse.value);
      auto second_parse = amanuensis::json::Reader::ParseString(first_write);
      REQUIRE_TRUE(second_parse.succeeded);
      std::string second_write = amanuensis::json::Writer::WriteToString(second_parse.value);

      ASSERT_EQUAL(first_write, second_write);
    });
  });

  DESCRIBE("Type preservation", {
    IT("preserves integer vs double distinction through round-trip", {
      std::string original_json = R"({"intValue":42,"doubleValue":3.14})";
      auto parsed = amanuensis::json::Reader::ParseString(original_json);
      REQUIRE_TRUE(parsed.succeeded);

      ASSERT_TRUE(parsed.value.Get("intValue").IsInteger());
      ASSERT_TRUE(parsed.value.Get("doubleValue").IsDouble());

      amanuensis::json::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string rewritten = amanuensis::json::Writer::WriteToString(parsed.value, minified_options);

      auto reparsed = amanuensis::json::Reader::ParseString(rewritten);
      REQUIRE_TRUE(reparsed.succeeded);
      ASSERT_TRUE(reparsed.value.Get("intValue").IsInteger());
      ASSERT_TRUE(reparsed.value.Get("doubleValue").IsDouble());
    });

    IT("preserves null through round-trip", {
      auto parsed = amanuensis::json::Reader::ParseString(R"({"v":null})");
      REQUIRE_TRUE(parsed.succeeded);

      amanuensis::json::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string rewritten = amanuensis::json::Writer::WriteToString(parsed.value, minified_options);

      auto reparsed = amanuensis::json::Reader::ParseString(rewritten);
      REQUIRE_TRUE(reparsed.succeeded);
      ASSERT_TRUE(reparsed.value.Get("v").IsNull());
    });
  });
});
