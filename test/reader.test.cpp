#include "cimmerian/test.hpp"
#include <amanuensis/io/parse-result.hpp>
#include <amanuensis/io/reader.hpp>
#include <amanuensis/io/writer.hpp>
#include <amanuensis/json.hpp>

#include <cmath>
#include <string>

// -----------------------------------------------------------------------
// Reader — parsing correctness
// -----------------------------------------------------------------------

DESCRIBE("Reader", {
  DESCRIBE("Literals", {
    IT("parses null", {
      auto result = amanuensis::Reader::ParseString("null");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(amanuensis::Json::IsNull(result.value));
    });

    IT("parses true", {
      auto result = amanuensis::Reader::ParseString("true");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(amanuensis::Json::IsBoolean(result.value));
      ASSERT_EQUAL(amanuensis::Json::AsBoolean(result.value), true);
    });

    IT("parses false", {
      auto result = amanuensis::Reader::ParseString("false");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(amanuensis::Json::IsBoolean(result.value));
      ASSERT_EQUAL(amanuensis::Json::AsBoolean(result.value), false);
    });
  });

  DESCRIBE("Numbers", {
    IT("parses a positive integer", {
      auto result = amanuensis::Reader::ParseString("42");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(amanuensis::Json::IsInteger(result.value));
      ASSERT_EQUAL(amanuensis::Json::AsInteger(result.value), 42LL);
    });

    IT("parses a negative integer", {
      auto result = amanuensis::Reader::ParseString("-7");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(amanuensis::Json::IsInteger(result.value));
      ASSERT_EQUAL(amanuensis::Json::AsInteger(result.value), -7LL);
    });

    IT("parses zero as integer", {
      auto result = amanuensis::Reader::ParseString("0");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(amanuensis::Json::IsInteger(result.value));
      ASSERT_EQUAL(amanuensis::Json::AsInteger(result.value), 0LL);
    });

    IT("parses a decimal number as double", {
      auto result = amanuensis::Reader::ParseString("3.14");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(amanuensis::Json::IsDouble(result.value));
      ASSERT_TRUE(std::abs(amanuensis::Json::AsDouble(result.value) - 3.14) < 1e-15);
    });

    IT("parses a number with exponent as double", {
      auto result = amanuensis::Reader::ParseString("1e10");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(amanuensis::Json::IsDouble(result.value));
    });

    IT("parses a number with negative exponent as double", {
      auto result = amanuensis::Reader::ParseString("5e-3");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(amanuensis::Json::IsDouble(result.value));
      ASSERT_TRUE(std::abs(amanuensis::Json::AsDouble(result.value) - 0.005) < 1e-15);
    });

    IT("parses a number with decimal and exponent as double", {
      auto result = amanuensis::Reader::ParseString("1.5e2");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(amanuensis::Json::IsDouble(result.value));
      ASSERT_TRUE(std::abs(amanuensis::Json::AsDouble(result.value) - 150.0) < 1e-10);
    });

    IT("rejects leading zeros", {
      auto result = amanuensis::Reader::ParseString("007");
      ASSERT_FALSE(result.succeeded);
    });

    IT("falls back to double on integer overflow", {
      auto result = amanuensis::Reader::ParseString("99999999999999999999999");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(amanuensis::Json::IsDouble(result.value));
    });
  });

  DESCRIBE("Strings", {
    IT("parses a simple string", {
      auto result = amanuensis::Reader::ParseString("\"hello world\"");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(amanuensis::Json::IsString(result.value));
      ASSERT_EQUAL(amanuensis::Json::AsString(result.value), std::string("hello world"));
    });

    IT("parses an empty string", {
      auto result = amanuensis::Reader::ParseString("\"\"");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(amanuensis::Json::AsString(result.value), std::string(""));
    });

    IT("parses escape sequence: newline", {
      auto result = amanuensis::Reader::ParseString("\"line1\\nline2\"");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(amanuensis::Json::AsString(result.value), std::string("line1\nline2"));
    });

    IT("parses escape sequence: tab", {
      auto result = amanuensis::Reader::ParseString("\"a\\tb\"");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(amanuensis::Json::AsString(result.value), std::string("a\tb"));
    });

    IT("parses escape sequence: backslash", {
      auto result = amanuensis::Reader::ParseString("\"a\\\\b\"");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(amanuensis::Json::AsString(result.value), std::string("a\\b"));
    });

    IT("parses escape sequence: double quote", {
      auto result = amanuensis::Reader::ParseString("\"say \\\"hi\\\"\"");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(amanuensis::Json::AsString(result.value), std::string("say \"hi\""));
    });

    IT("parses escape sequence: forward slash", {
      auto result = amanuensis::Reader::ParseString("\"a\\/b\"");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(amanuensis::Json::AsString(result.value), std::string("a/b"));
    });

    IT("parses Unicode escape \\u0041 as 'A'", {
      auto result = amanuensis::Reader::ParseString("\"\\u0041\"");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(amanuensis::Json::AsString(result.value), std::string("A"));
    });

    IT("parses Unicode surrogate pair for emoji", {
      auto result = amanuensis::Reader::ParseString("\"\\uD83D\\uDE00\"");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(amanuensis::Json::AsString(result.value).size(), 4u);
    });
  });

  DESCRIBE("Arrays", {
    IT("parses an empty array", {
      auto result = amanuensis::Reader::ParseString("[]");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(amanuensis::Json::IsArray(result.value));
      ASSERT_EQUAL(amanuensis::Json::Size(result.value), 0u);
    });

    IT("parses an array of integers", {
      auto result = amanuensis::Reader::ParseString("[1, 2, 3]");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(amanuensis::Json::Size(result.value), 3u);
      ASSERT_EQUAL(amanuensis::Json::AsInteger(amanuensis::Json::At(result.value, 0)), 1LL);
      ASSERT_EQUAL(amanuensis::Json::AsInteger(amanuensis::Json::At(result.value, 2)), 3LL);
    });

    IT("parses an array of mixed types", {
      auto result = amanuensis::Reader::ParseString("[1, \"two\", true, null]");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(amanuensis::Json::Size(result.value), 4u);
      ASSERT_TRUE(amanuensis::Json::IsInteger(amanuensis::Json::At(result.value, 0)));
      ASSERT_TRUE(amanuensis::Json::IsString(amanuensis::Json::At(result.value, 1)));
      ASSERT_TRUE(amanuensis::Json::IsBoolean(amanuensis::Json::At(result.value, 2)));
      ASSERT_TRUE(amanuensis::Json::IsNull(amanuensis::Json::At(result.value, 3)));
    });

    IT("parses nested arrays", {
      auto result = amanuensis::Reader::ParseString("[[1, 2], [3, 4]]");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(amanuensis::Json::Size(result.value), 2u);
      ASSERT_EQUAL(amanuensis::Json::AsInteger(amanuensis::Json::At(amanuensis::Json::At(result.value, 0), 1)), 2LL);
      ASSERT_EQUAL(amanuensis::Json::AsInteger(amanuensis::Json::At(amanuensis::Json::At(result.value, 1), 0)), 3LL);
    });
  });

  DESCRIBE("Objects", {
    IT("parses an empty object", {
      auto result = amanuensis::Reader::ParseString("{}");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(amanuensis::Json::IsObject(result.value));
      ASSERT_EQUAL(amanuensis::Json::Size(result.value), 0u);
    });

    IT("parses an object with string and integer values", {
      auto result = amanuensis::Reader::ParseString("{\"x\": 10, \"y\": 20}");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(amanuensis::Json::AsInteger(amanuensis::Json::Get(result.value, "x")), 10LL);
      ASSERT_EQUAL(amanuensis::Json::AsInteger(amanuensis::Json::Get(result.value, "y")), 20LL);
    });

    IT("parses a deeply nested structure", {
      auto result = amanuensis::Reader::ParseString("{\"a\": {\"b\": {\"c\": [1, {\"d\": true}]}}}");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(amanuensis::Json::AsBoolean(
        amanuensis::Json::Get(
          amanuensis::Json::At(
            amanuensis::Json::Get(
              amanuensis::Json::Get(
                amanuensis::Json::Get(result.value, "a"),
              "b"),
            "c"),
          1),
        "d")
      ));
    });
  });

  DESCRIBE("Error reporting", {
    IT("fails on malformed object", {
      auto result = amanuensis::Reader::ParseString("{\"a\": }");
      ASSERT_FALSE(result.succeeded);
      ASSERT_EQUAL(result.error.line, 1);
      ASSERT_TRUE(result.error.column > 0);
      ASSERT_FALSE(result.error.message.empty());
    });

    IT("rejects trailing comma in array", {
      auto result = amanuensis::Reader::ParseString("[1, 2, ]");
      ASSERT_FALSE(result.succeeded);
    });

    IT("rejects trailing comma in object", {
      auto result = amanuensis::Reader::ParseString("{\"a\": 1, }");
      ASSERT_FALSE(result.succeeded);
    });

    IT("rejects trailing content after valid JSON", {
      auto result = amanuensis::Reader::ParseString("42 extra");
      ASSERT_FALSE(result.succeeded);
    });

    IT("reports correct line number on multiline input", {
      auto result = amanuensis::Reader::ParseString("{\n  \"a\": \n  bad\n}");
      ASSERT_FALSE(result.succeeded);
      ASSERT_EQUAL(result.error.line, 3);
    });

    IT("rejects unterminated string", {
      auto result = amanuensis::Reader::ParseString("\"hello");
      ASSERT_FALSE(result.succeeded);
    });

    IT("rejects invalid escape sequence", {
      auto result = amanuensis::Reader::ParseString("\"\\q\"");
      ASSERT_FALSE(result.succeeded);
    });

    IT("rejects unescaped control character in string", {
      std::string input = "\"hello\x01world\"";
      auto result = amanuensis::Reader::ParseString(input);
      ASSERT_FALSE(result.succeeded);
    });

    IT("rejects unexpected lone low surrogate", {
      auto result = amanuensis::Reader::ParseString("\"\\uDC00\"");
      ASSERT_FALSE(result.succeeded);
    });

    IT("fails on empty input", {
      auto result = amanuensis::Reader::ParseString("");
      ASSERT_FALSE(result.succeeded);
    });
  });

  DESCRIBE("File I/O", {
    IT("reads and parses a JSON file", {
      amanuensis::Value object_value = amanuensis::Json::MakeObject();
      amanuensis::Json::Insert(object_value, "fileTest", amanuensis::Value{ true });
      bool write_succeeded =
          amanuensis::Writer::WriteToFile(object_value, "/tmp/amanuensis_reader_test.json");
      REQUIRE_TRUE(write_succeeded);

      auto result = amanuensis::Reader::ParseFile("/tmp/amanuensis_reader_test.json");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(amanuensis::Json::AsBoolean(amanuensis::Json::Get(result.value, "fileTest")));
    });

    IT("returns error for nonexistent file", {
      auto result = amanuensis::Reader::ParseFile("/tmp/no_such_file_amanuensis.json");
      ASSERT_FALSE(result.succeeded);
    });
  });
});
