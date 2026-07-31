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
      auto result = Amanuensis::Reader::ParseString("null");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(Amanuensis::Json::IsNull(result.value));
    });

    IT("parses true", {
      auto result = Amanuensis::Reader::ParseString("true");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(Amanuensis::Json::IsBoolean(result.value));
      ASSERT_EQUAL(Amanuensis::Json::AsBoolean(result.value), true);
    });

    IT("parses false", {
      auto result = Amanuensis::Reader::ParseString("false");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(Amanuensis::Json::IsBoolean(result.value));
      ASSERT_EQUAL(Amanuensis::Json::AsBoolean(result.value), false);
    });
  });

  DESCRIBE("Numbers", {
    IT("parses a positive integer", {
      auto result = Amanuensis::Reader::ParseString("42");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(Amanuensis::Json::IsInteger(result.value));
      ASSERT_EQUAL(Amanuensis::Json::AsInteger(result.value), 42LL);
    });

    IT("parses a negative integer", {
      auto result = Amanuensis::Reader::ParseString("-7");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(Amanuensis::Json::IsInteger(result.value));
      ASSERT_EQUAL(Amanuensis::Json::AsInteger(result.value), -7LL);
    });

    IT("parses zero as integer", {
      auto result = Amanuensis::Reader::ParseString("0");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(Amanuensis::Json::IsInteger(result.value));
      ASSERT_EQUAL(Amanuensis::Json::AsInteger(result.value), 0LL);
    });

    IT("parses a decimal number as double", {
      auto result = Amanuensis::Reader::ParseString("3.14");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(Amanuensis::Json::IsDouble(result.value));
      ASSERT_TRUE(std::abs(Amanuensis::Json::AsDouble(result.value) - 3.14) < 1e-15);
    });

    IT("parses a number with exponent as double", {
      auto result = Amanuensis::Reader::ParseString("1e10");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(Amanuensis::Json::IsDouble(result.value));
    });

    IT("parses a number with negative exponent as double", {
      auto result = Amanuensis::Reader::ParseString("5e-3");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(Amanuensis::Json::IsDouble(result.value));
      ASSERT_TRUE(std::abs(Amanuensis::Json::AsDouble(result.value) - 0.005) < 1e-15);
    });

    IT("parses a number with decimal and exponent as double", {
      auto result = Amanuensis::Reader::ParseString("1.5e2");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(Amanuensis::Json::IsDouble(result.value));
      ASSERT_TRUE(std::abs(Amanuensis::Json::AsDouble(result.value) - 150.0) < 1e-10);
    });

    IT("rejects leading zeros", {
      auto result = Amanuensis::Reader::ParseString("007");
      ASSERT_FALSE(result.succeeded);
    });

    IT("falls back to double on integer overflow", {
      auto result = Amanuensis::Reader::ParseString("99999999999999999999999");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(Amanuensis::Json::IsDouble(result.value));
    });
  });

  DESCRIBE("Strings", {
    IT("parses a simple string", {
      auto result = Amanuensis::Reader::ParseString("\"hello world\"");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(Amanuensis::Json::IsString(result.value));
      ASSERT_EQUAL(Amanuensis::Json::AsString(result.value), std::string("hello world"));
    });

    IT("parses an empty string", {
      auto result = Amanuensis::Reader::ParseString("\"\"");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(Amanuensis::Json::AsString(result.value), std::string(""));
    });

    IT("parses escape sequence: newline", {
      auto result = Amanuensis::Reader::ParseString("\"line1\\nline2\"");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(Amanuensis::Json::AsString(result.value), std::string("line1\nline2"));
    });

    IT("parses escape sequence: tab", {
      auto result = Amanuensis::Reader::ParseString("\"a\\tb\"");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(Amanuensis::Json::AsString(result.value), std::string("a\tb"));
    });

    IT("parses escape sequence: backslash", {
      auto result = Amanuensis::Reader::ParseString("\"a\\\\b\"");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(Amanuensis::Json::AsString(result.value), std::string("a\\b"));
    });

    IT("parses escape sequence: double quote", {
      auto result = Amanuensis::Reader::ParseString("\"say \\\"hi\\\"\"");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(Amanuensis::Json::AsString(result.value), std::string("say \"hi\""));
    });

    IT("parses escape sequence: forward slash", {
      auto result = Amanuensis::Reader::ParseString("\"a\\/b\"");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(Amanuensis::Json::AsString(result.value), std::string("a/b"));
    });

    IT("parses Unicode escape \\u0041 as 'A'", {
      auto result = Amanuensis::Reader::ParseString("\"\\u0041\"");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(Amanuensis::Json::AsString(result.value), std::string("A"));
    });

    IT("parses Unicode surrogate pair for emoji", {
      auto result = Amanuensis::Reader::ParseString("\"\\uD83D\\uDE00\"");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(Amanuensis::Json::AsString(result.value).size(), 4u);
    });
  });

  DESCRIBE("Arrays", {
    IT("parses an empty array", {
      auto result = Amanuensis::Reader::ParseString("[]");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(Amanuensis::Json::IsArray(result.value));
      ASSERT_EQUAL(Amanuensis::Json::Size(result.value), 0u);
    });

    IT("parses an array of integers", {
      auto result = Amanuensis::Reader::ParseString("[1, 2, 3]");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(Amanuensis::Json::Size(result.value), 3u);
      ASSERT_EQUAL(Amanuensis::Json::AsInteger(Amanuensis::Json::At(result.value, 0)), 1LL);
      ASSERT_EQUAL(Amanuensis::Json::AsInteger(Amanuensis::Json::At(result.value, 2)), 3LL);
    });

    IT("parses an array of mixed types", {
      auto result = Amanuensis::Reader::ParseString("[1, \"two\", true, null]");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(Amanuensis::Json::Size(result.value), 4u);
      ASSERT_TRUE(Amanuensis::Json::IsInteger(Amanuensis::Json::At(result.value, 0)));
      ASSERT_TRUE(Amanuensis::Json::IsString(Amanuensis::Json::At(result.value, 1)));
      ASSERT_TRUE(Amanuensis::Json::IsBoolean(Amanuensis::Json::At(result.value, 2)));
      ASSERT_TRUE(Amanuensis::Json::IsNull(Amanuensis::Json::At(result.value, 3)));
    });

    IT("parses nested arrays", {
      auto result = Amanuensis::Reader::ParseString("[[1, 2], [3, 4]]");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(Amanuensis::Json::Size(result.value), 2u);
      ASSERT_EQUAL(Amanuensis::Json::AsInteger(Amanuensis::Json::At(Amanuensis::Json::At(result.value, 0), 1)), 2LL);
      ASSERT_EQUAL(Amanuensis::Json::AsInteger(Amanuensis::Json::At(Amanuensis::Json::At(result.value, 1), 0)), 3LL);
    });
  });

  DESCRIBE("Objects", {
    IT("parses an empty object", {
      auto result = Amanuensis::Reader::ParseString("{}");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(Amanuensis::Json::IsObject(result.value));
      ASSERT_EQUAL(Amanuensis::Json::Size(result.value), 0u);
    });

    IT("parses an object with string and integer values", {
      auto result = Amanuensis::Reader::ParseString("{\"x\": 10, \"y\": 20}");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(Amanuensis::Json::AsInteger(Amanuensis::Json::Get(result.value, "x")), 10LL);
      ASSERT_EQUAL(Amanuensis::Json::AsInteger(Amanuensis::Json::Get(result.value, "y")), 20LL);
    });

    IT("parses a deeply nested structure", {
      auto result = Amanuensis::Reader::ParseString("{\"a\": {\"b\": {\"c\": [1, {\"d\": true}]}}}");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(Amanuensis::Json::AsBoolean(
        Amanuensis::Json::Get(
          Amanuensis::Json::At(
            Amanuensis::Json::Get(
              Amanuensis::Json::Get(
                Amanuensis::Json::Get(result.value, "a"),
              "b"),
            "c"),
          1),
        "d")
      ));
    });
  });

  DESCRIBE("Error reporting", {
    IT("fails on malformed object", {
      auto result = Amanuensis::Reader::ParseString("{\"a\": }");
      ASSERT_FALSE(result.succeeded);
      ASSERT_EQUAL(result.error.line, 1);
      ASSERT_TRUE(result.error.column > 0);
      ASSERT_FALSE(result.error.message.empty());
    });

    IT("rejects trailing comma in array", {
      auto result = Amanuensis::Reader::ParseString("[1, 2, ]");
      ASSERT_FALSE(result.succeeded);
    });

    IT("rejects trailing comma in object", {
      auto result = Amanuensis::Reader::ParseString("{\"a\": 1, }");
      ASSERT_FALSE(result.succeeded);
    });

    IT("rejects trailing content after valid JSON", {
      auto result = Amanuensis::Reader::ParseString("42 extra");
      ASSERT_FALSE(result.succeeded);
    });

    IT("reports correct line number on multiline input", {
      auto result = Amanuensis::Reader::ParseString("{\n  \"a\": \n  bad\n}");
      ASSERT_FALSE(result.succeeded);
      ASSERT_EQUAL(result.error.line, 3);
    });

    IT("rejects unterminated string", {
      auto result = Amanuensis::Reader::ParseString("\"hello");
      ASSERT_FALSE(result.succeeded);
    });

    IT("rejects invalid escape sequence", {
      auto result = Amanuensis::Reader::ParseString("\"\\q\"");
      ASSERT_FALSE(result.succeeded);
    });

    IT("rejects unescaped control character in string", {
      std::string input = "\"hello\x01world\"";
      auto result = Amanuensis::Reader::ParseString(input);
      ASSERT_FALSE(result.succeeded);
    });

    IT("rejects unexpected lone low surrogate", {
      auto result = Amanuensis::Reader::ParseString("\"\\uDC00\"");
      ASSERT_FALSE(result.succeeded);
    });

    IT("fails on empty input", {
      auto result = Amanuensis::Reader::ParseString("");
      ASSERT_FALSE(result.succeeded);
    });
  });

  DESCRIBE("File I/O", {
    IT("reads and parses a JSON file", {
      Amanuensis::Value object_value = Amanuensis::Json::MakeObject();
      Amanuensis::Json::Insert(object_value, "fileTest", Amanuensis::Value{ true });
      bool write_succeeded =
          Amanuensis::Writer::WriteToFile(object_value, "/tmp/amanuensis_reader_test.json");
      REQUIRE_TRUE(write_succeeded);

      auto result = Amanuensis::Reader::ParseFile("/tmp/amanuensis_reader_test.json");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(Amanuensis::Json::AsBoolean(Amanuensis::Json::Get(result.value, "fileTest")));
    });

    IT("returns error for nonexistent file", {
      auto result = Amanuensis::Reader::ParseFile("/tmp/no_such_file_amanuensis.json");
      ASSERT_FALSE(result.succeeded);
    });
  });
});
