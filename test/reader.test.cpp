#include "cimmerian/test.hpp"
#include <amanuensis/io/parser-result.hpp>
#include <amanuensis/io/reader.hpp>
#include <amanuensis/io/writer.hpp>

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
      ASSERT_TRUE(result.value.IsNull());
    });

    IT("parses true", {
      auto result = Amanuensis::Reader::ParseString("true");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(result.value.IsBoolean());
      ASSERT_EQUAL(result.value.AsBoolean(), true);
    });

    IT("parses false", {
      auto result = Amanuensis::Reader::ParseString("false");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(result.value.IsBoolean());
      ASSERT_EQUAL(result.value.AsBoolean(), false);
    });
  });

  DESCRIBE("Numbers", {
    IT("parses a positive integer", {
      auto result = Amanuensis::Reader::ParseString("42");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(result.value.IsInteger());
      ASSERT_EQUAL(result.value.AsInteger(), 42LL);
    });

    IT("parses a negative integer", {
      auto result = Amanuensis::Reader::ParseString("-7");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(result.value.IsInteger());
      ASSERT_EQUAL(result.value.AsInteger(), -7LL);
    });

    IT("parses zero as integer", {
      auto result = Amanuensis::Reader::ParseString("0");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(result.value.IsInteger());
      ASSERT_EQUAL(result.value.AsInteger(), 0LL);
    });

    IT("parses a decimal number as double", {
      auto result = Amanuensis::Reader::ParseString("3.14");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(result.value.IsDouble());
      ASSERT_TRUE(std::abs(result.value.AsDouble() - 3.14) < 1e-15);
    });

    IT("parses a number with exponent as double", {
      auto result = Amanuensis::Reader::ParseString("1e10");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(result.value.IsDouble());
    });

    IT("parses a number with negative exponent as double", {
      auto result = Amanuensis::Reader::ParseString("5e-3");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(result.value.IsDouble());
      ASSERT_TRUE(std::abs(result.value.AsDouble() - 0.005) < 1e-15);
    });

    IT("parses a number with decimal and exponent as double", {
      auto result = Amanuensis::Reader::ParseString("1.5e2");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(result.value.IsDouble());
      ASSERT_TRUE(std::abs(result.value.AsDouble() - 150.0) < 1e-10);
    });

    IT("rejects leading zeros", {
      auto result = Amanuensis::Reader::ParseString("007");
      ASSERT_FALSE(result.succeeded);
    });

    IT("falls back to double on integer overflow", {
      // A number larger than LLONG_MAX
      auto result = Amanuensis::Reader::ParseString("99999999999999999999999");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(result.value.IsDouble());
    });
  });

  DESCRIBE("Strings", {
    IT("parses a simple string", {
      auto result = Amanuensis::Reader::ParseString("\"hello world\"");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(result.value.IsString());
      ASSERT_EQUAL(result.value.AsString(), std::string("hello world"));
    });

    IT("parses an empty string", {
      auto result = Amanuensis::Reader::ParseString("\"\"");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(result.value.AsString(), std::string(""));
    });

    IT("parses escape sequence: newline", {
      auto result = Amanuensis::Reader::ParseString("\"line1\\nline2\"");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(result.value.AsString(), std::string("line1\nline2"));
    });

    IT("parses escape sequence: tab", {
      auto result = Amanuensis::Reader::ParseString("\"a\\tb\"");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(result.value.AsString(), std::string("a\tb"));
    });

    IT("parses escape sequence: backslash", {
      auto result = Amanuensis::Reader::ParseString("\"a\\\\b\"");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(result.value.AsString(), std::string("a\\b"));
    });

    IT("parses escape sequence: double quote", {
      auto result = Amanuensis::Reader::ParseString("\"say \\\"hi\\\"\"");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(result.value.AsString(), std::string("say \"hi\""));
    });

    IT("parses escape sequence: forward slash", {
      auto result = Amanuensis::Reader::ParseString("\"a\\/b\"");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(result.value.AsString(), std::string("a/b"));
    });

    IT("parses Unicode escape \\u0041 as 'A'", {
      auto result = Amanuensis::Reader::ParseString("\"\\u0041\"");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(result.value.AsString(), std::string("A"));
    });

    IT("parses Unicode surrogate pair for emoji", {
      // U+1F600 = \uD83D\uDE00
      auto result = Amanuensis::Reader::ParseString("\"\\uD83D\\uDE00\"");
      ASSERT_TRUE(result.succeeded);
      // Should be the UTF-8 encoding of U+1F600
      ASSERT_EQUAL(result.value.AsString().size(), 4u);
    });
  });

  DESCRIBE("Arrays", {
    IT("parses an empty array", {
      auto result = Amanuensis::Reader::ParseString("[]");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(result.value.IsArray());
      ASSERT_EQUAL(result.value.Size(), 0u);
    });

    IT("parses an array of integers", {
      auto result = Amanuensis::Reader::ParseString("[1, 2, 3]");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(result.value.Size(), 3u);
      ASSERT_EQUAL(result.value.At(0).AsInteger(), 1LL);
      ASSERT_EQUAL(result.value.At(2).AsInteger(), 3LL);
    });

    IT("parses an array of mixed types", {
      auto result = Amanuensis::Reader::ParseString("[1, \"two\", true, null]");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(result.value.Size(), 4u);
      ASSERT_TRUE(result.value.At(0).IsInteger());
      ASSERT_TRUE(result.value.At(1).IsString());
      ASSERT_TRUE(result.value.At(2).IsBoolean());
      ASSERT_TRUE(result.value.At(3).IsNull());
    });

    IT("parses nested arrays", {
      auto result = Amanuensis::Reader::ParseString("[[1, 2], [3, 4]]");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(result.value.Size(), 2u);
      ASSERT_EQUAL(result.value.At(0).At(1).AsInteger(), 2LL);
      ASSERT_EQUAL(result.value.At(1).At(0).AsInteger(), 3LL);
    });
  });

  DESCRIBE("Objects", {
    IT("parses an empty object", {
      auto result = Amanuensis::Reader::ParseString("{}");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(result.value.IsObject());
      ASSERT_EQUAL(result.value.Size(), 0u);
    });

    IT("parses an object with string and integer values", {
      auto result = Amanuensis::Reader::ParseString("{\"x\": 10, \"y\": 20}");
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(result.value.Get("x").AsInteger(), 10LL);
      ASSERT_EQUAL(result.value.Get("y").AsInteger(), 20LL);
    });

    IT("parses a deeply nested structure", {
      auto result =
          Amanuensis::Reader::ParseString("{\"a\": {\"b\": {\"c\": [1, {\"d\": true}]}}}");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(result.value.Get("a").Get("b").Get("c").At(1).Get("d").AsBoolean());
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
      // Write a file, then read it back
      Amanuensis::Value objectValue = Amanuensis::Value::MakeObject();
      objectValue.Insert("fileTest", Amanuensis::Value(true));
      bool writeSucceeded =
          Amanuensis::Writer::WriteToFile(objectValue, "/tmp/amanuensis_reader_test.json");
      REQUIRE_TRUE(writeSucceeded);

      auto result = Amanuensis::Reader::ParseFile("/tmp/amanuensis_reader_test.json");
      ASSERT_TRUE(result.succeeded);
      ASSERT_TRUE(result.value.Get("fileTest").AsBoolean());
    });

    IT("returns error for nonexistent file", {
      auto result = Amanuensis::Reader::ParseFile("/tmp/no_such_file_amanuensis.json");
      ASSERT_FALSE(result.succeeded);
    });
  });
});
