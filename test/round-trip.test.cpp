#include "cimmerian/test.hpp"
#include <amanuensis/io/reader.hpp>
#include <amanuensis/io/writer.hpp>
#include <amanuensis/io/parser-result.hpp>

#include <string>

// -----------------------------------------------------------------------
// Round-trip — parse → write → parse produces same structure
// -----------------------------------------------------------------------

DESCRIBE("Round-trip", {
  DESCRIBE("Minified round-trip", {
    IT("round-trips a flat object", {
      std::string originalJson = R"({"name":"test","count":42,"flag":true})";
      auto firstParse = Amanuensis::Reader::ParseString(originalJson);
      REQUIRE_TRUE(firstParse.succeeded);

      Amanuensis::WriterOptions minifiedOptions;
      minifiedOptions.pretty = false;
      minifiedOptions.trailingNewline = false;
      std::string rewritten = Amanuensis::Writer::WriteToString(firstParse.value, minifiedOptions);

      auto secondParse = Amanuensis::Reader::ParseString(rewritten);
      ASSERT_TRUE(secondParse.succeeded);
      ASSERT_EQUAL(secondParse.value.Get("name").AsString(), std::string("test"));
      ASSERT_EQUAL(secondParse.value.Get("count").AsInteger(), 42LL);
      ASSERT_TRUE(secondParse.value.Get("flag").AsBoolean());
    });

    IT("round-trips nested objects and arrays", {
      std::string originalJson = R"({"name":"test","values":[1,2,3],"nested":{"flag":true}})";
      auto firstParse = Amanuensis::Reader::ParseString(originalJson);
      REQUIRE_TRUE(firstParse.succeeded);

      Amanuensis::WriterOptions minifiedOptions;
      minifiedOptions.pretty = false;
      minifiedOptions.trailingNewline = false;
      std::string rewritten = Amanuensis::Writer::WriteToString(firstParse.value, minifiedOptions);

      auto secondParse = Amanuensis::Reader::ParseString(rewritten);
      ASSERT_TRUE(secondParse.succeeded);
      ASSERT_EQUAL(secondParse.value.Get("name").AsString(), std::string("test"));
      ASSERT_EQUAL(secondParse.value.Get("values").Size(), 3u);
      ASSERT_TRUE(secondParse.value.Get("nested").Get("flag").AsBoolean());
    });

    IT("round-trips strings with escape sequences", {
      std::string originalJson = R"({"text":"line1\nline2\ttab\\slash\"quote"})";
      auto firstParse = Amanuensis::Reader::ParseString(originalJson);
      REQUIRE_TRUE(firstParse.succeeded);

      Amanuensis::WriterOptions minifiedOptions;
      minifiedOptions.pretty = false;
      minifiedOptions.trailingNewline = false;
      std::string rewritten = Amanuensis::Writer::WriteToString(firstParse.value, minifiedOptions);

      auto secondParse = Amanuensis::Reader::ParseString(rewritten);
      ASSERT_TRUE(secondParse.succeeded);
      ASSERT_EQUAL(
          secondParse.value.Get("text").AsString(), firstParse.value.Get("text").AsString()
      );
    });
  });

  DESCRIBE("Pretty round-trip", {
    IT("pretty-printed output parses back to the same structure", {
      std::string originalJson = R"({"a":1,"b":[2,3],"c":{"d":true}})";
      auto firstParse = Amanuensis::Reader::ParseString(originalJson);
      REQUIRE_TRUE(firstParse.succeeded);

      // Write pretty
      std::string prettyOutput = Amanuensis::Writer::WriteToString(firstParse.value);

      // Parse back
      auto secondParse = Amanuensis::Reader::ParseString(prettyOutput);
      ASSERT_TRUE(secondParse.succeeded);
      ASSERT_EQUAL(secondParse.value.Get("a").AsInteger(), 1LL);
      ASSERT_EQUAL(secondParse.value.Get("b").Size(), 2u);
      ASSERT_TRUE(secondParse.value.Get("c").Get("d").AsBoolean());
    });

    IT("second pretty write is byte-identical to first", {
      std::string originalJson = R"({"a":1,"b":[2,3],"c":{"d":"hello"}})";
      auto firstParse = Amanuensis::Reader::ParseString(originalJson);
      REQUIRE_TRUE(firstParse.succeeded);

      std::string firstWrite = Amanuensis::Writer::WriteToString(firstParse.value);
      auto secondParse = Amanuensis::Reader::ParseString(firstWrite);
      REQUIRE_TRUE(secondParse.succeeded);
      std::string secondWrite = Amanuensis::Writer::WriteToString(secondParse.value);

      ASSERT_EQUAL(firstWrite, secondWrite);
    });
  });

  DESCRIBE("Type preservation", {
    IT("preserves integer vs double distinction through round-trip", {
      std::string originalJson = R"({"intValue":42,"doubleValue":3.14})";
      auto parsed = Amanuensis::Reader::ParseString(originalJson);
      REQUIRE_TRUE(parsed.succeeded);

      ASSERT_TRUE(parsed.value.Get("intValue").IsInteger());
      ASSERT_TRUE(parsed.value.Get("doubleValue").IsDouble());

      Amanuensis::WriterOptions minifiedOptions;
      minifiedOptions.pretty = false;
      minifiedOptions.trailingNewline = false;
      std::string rewritten = Amanuensis::Writer::WriteToString(parsed.value, minifiedOptions);

      auto reparsed = Amanuensis::Reader::ParseString(rewritten);
      REQUIRE_TRUE(reparsed.succeeded);
      ASSERT_TRUE(reparsed.value.Get("intValue").IsInteger());
      ASSERT_TRUE(reparsed.value.Get("doubleValue").IsDouble());
    });

    IT("preserves null through round-trip", {
      auto parsed = Amanuensis::Reader::ParseString(R"({"v":null})");
      REQUIRE_TRUE(parsed.succeeded);

      Amanuensis::WriterOptions minifiedOptions;
      minifiedOptions.pretty = false;
      minifiedOptions.trailingNewline = false;
      std::string rewritten = Amanuensis::Writer::WriteToString(parsed.value, minifiedOptions);

      auto reparsed = Amanuensis::Reader::ParseString(rewritten);
      REQUIRE_TRUE(reparsed.succeeded);
      ASSERT_TRUE(reparsed.value.Get("v").IsNull());
    });
  });
});
