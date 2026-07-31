#include "cimmerian/test.hpp"
#include <amanuensis/io/reader.hpp>
#include <amanuensis/io/writer.hpp>
#include <amanuensis/io/parse-result.hpp>
#include <amanuensis/json.hpp>

#include <string>

// -----------------------------------------------------------------------
// Round-trip — parse → write → parse produces same structure
// -----------------------------------------------------------------------

DESCRIBE("Round-trip", {
  DESCRIBE("Minified round-trip", {
    IT("round-trips a flat object", {
      std::string original_json = R"({"name":"test","count":42,"flag":true})";
      auto first_parse = Amanuensis::Reader::ParseString(original_json);
      REQUIRE_TRUE(first_parse.succeeded);

      Amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string rewritten = Amanuensis::Writer::WriteToString(first_parse.value, minified_options);

      auto second_parse = Amanuensis::Reader::ParseString(rewritten);
      ASSERT_TRUE(second_parse.succeeded);
      ASSERT_EQUAL(Amanuensis::Json::AsString(Amanuensis::Json::Get(second_parse.value, "name")), std::string("test"));
      ASSERT_EQUAL(Amanuensis::Json::AsInteger(Amanuensis::Json::Get(second_parse.value, "count")), 42LL);
      ASSERT_TRUE(Amanuensis::Json::AsBoolean(Amanuensis::Json::Get(second_parse.value, "flag")));
    });

    IT("round-trips nested objects and arrays", {
      std::string original_json = R"({"name":"test","values":[1,2,3],"nested":{"flag":true}})";
      auto first_parse = Amanuensis::Reader::ParseString(original_json);
      REQUIRE_TRUE(first_parse.succeeded);

      Amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string rewritten = Amanuensis::Writer::WriteToString(first_parse.value, minified_options);

      auto second_parse = Amanuensis::Reader::ParseString(rewritten);
      ASSERT_TRUE(second_parse.succeeded);
      ASSERT_EQUAL(Amanuensis::Json::AsString(Amanuensis::Json::Get(second_parse.value, "name")), std::string("test"));
      ASSERT_EQUAL(Amanuensis::Json::Size(Amanuensis::Json::Get(second_parse.value, "values")), 3u);
      ASSERT_TRUE(Amanuensis::Json::AsBoolean(Amanuensis::Json::Get(Amanuensis::Json::Get(second_parse.value, "nested"), "flag")));
    });

    IT("round-trips strings with escape sequences", {
      std::string original_json = R"({"text":"line1\nline2\ttab\\slash\"quote"})";
      auto first_parse = Amanuensis::Reader::ParseString(original_json);
      REQUIRE_TRUE(first_parse.succeeded);

      Amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string rewritten = Amanuensis::Writer::WriteToString(first_parse.value, minified_options);

      auto second_parse = Amanuensis::Reader::ParseString(rewritten);
      ASSERT_TRUE(second_parse.succeeded);
      ASSERT_EQUAL(
        Amanuensis::Json::AsString(Amanuensis::Json::Get(second_parse.value, "text")),
        Amanuensis::Json::AsString(Amanuensis::Json::Get(first_parse.value, "text"))
      );
    });
  });

  DESCRIBE("Pretty round-trip", {
    IT("pretty-printed output parses back to the same structure", {
      std::string original_json = R"({"a":1,"b":[2,3],"c":{"d":true}})";
      auto first_parse = Amanuensis::Reader::ParseString(original_json);
      REQUIRE_TRUE(first_parse.succeeded);

      std::string pretty_output = Amanuensis::Writer::WriteToString(first_parse.value);

      auto second_parse = Amanuensis::Reader::ParseString(pretty_output);
      ASSERT_TRUE(second_parse.succeeded);
      ASSERT_EQUAL(Amanuensis::Json::AsInteger(Amanuensis::Json::Get(second_parse.value, "a")), 1LL);
      ASSERT_EQUAL(Amanuensis::Json::Size(Amanuensis::Json::Get(second_parse.value, "b")), 2u);
      ASSERT_TRUE(Amanuensis::Json::AsBoolean(Amanuensis::Json::Get(Amanuensis::Json::Get(second_parse.value, "c"), "d")));
    });

    IT("second pretty write is byte-identical to first", {
      std::string original_json = R"({"a":1,"b":[2,3],"c":{"d":"hello"}})";
      auto first_parse = Amanuensis::Reader::ParseString(original_json);
      REQUIRE_TRUE(first_parse.succeeded);

      std::string first_write = Amanuensis::Writer::WriteToString(first_parse.value);
      auto second_parse = Amanuensis::Reader::ParseString(first_write);
      REQUIRE_TRUE(second_parse.succeeded);
      std::string second_write = Amanuensis::Writer::WriteToString(second_parse.value);

      ASSERT_EQUAL(first_write, second_write);
    });
  });

  DESCRIBE("Type preservation", {
    IT("preserves integer vs double distinction through round-trip", {
      std::string original_json = R"({"intValue":42,"doubleValue":3.14})";
      auto parsed = Amanuensis::Reader::ParseString(original_json);
      REQUIRE_TRUE(parsed.succeeded);

      ASSERT_TRUE(Amanuensis::Json::IsInteger(Amanuensis::Json::Get(parsed.value, "intValue")));
      ASSERT_TRUE(Amanuensis::Json::IsDouble(Amanuensis::Json::Get(parsed.value, "doubleValue")));

      Amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string rewritten = Amanuensis::Writer::WriteToString(parsed.value, minified_options);

      auto reparsed = Amanuensis::Reader::ParseString(rewritten);
      REQUIRE_TRUE(reparsed.succeeded);
      ASSERT_TRUE(Amanuensis::Json::IsInteger(Amanuensis::Json::Get(reparsed.value, "intValue")));
      ASSERT_TRUE(Amanuensis::Json::IsDouble(Amanuensis::Json::Get(reparsed.value, "doubleValue")));
    });

    IT("preserves null through round-trip", {
      auto parsed = Amanuensis::Reader::ParseString(R"({"v":null})");
      REQUIRE_TRUE(parsed.succeeded);

      Amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string rewritten = Amanuensis::Writer::WriteToString(parsed.value, minified_options);

      auto reparsed = Amanuensis::Reader::ParseString(rewritten);
      REQUIRE_TRUE(reparsed.succeeded);
      ASSERT_TRUE(Amanuensis::Json::IsNull(Amanuensis::Json::Get(reparsed.value, "v")));
    });
  });
});
