#include "cimmerian/test.hpp"
#include <amanuensis/json-value.hpp>
#include <amanuensis/json.hpp>
#include <amanuensis/io/json-parse-result.hpp>
#include <amanuensis/io/writer.hpp>
#include <amanuensis/io/reader.hpp>

#include <string>
#include <variant>

DESCRIBE("Writer", {
  DESCRIBE("Null output", {
    IT("writes null", {
      amanuensis::JsonValue null_value{std::monostate()};
      amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output = amanuensis::Writer::WriteToString(null_value, minified_options);
      ASSERT_EQUAL(output, std::string("null"));
    });
  });

  DESCRIBE("Boolean output", {
    IT("writes true", {
      amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output =
          amanuensis::Writer::WriteToString(amanuensis::JsonValue{true}, minified_options);
      ASSERT_EQUAL(output, std::string("true"));
    });

    IT("writes false", {
      amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output =
          amanuensis::Writer::WriteToString(amanuensis::JsonValue{false}, minified_options);
      ASSERT_EQUAL(output, std::string("false"));
    });
  });

  DESCRIBE("Integer output", {
    IT("writes a positive integer without decimal point", {
      amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output =
          amanuensis::Writer::WriteToString(amanuensis::JsonValue{42LL}, minified_options);
      ASSERT_EQUAL(output, std::string("42"));
    });

    IT("writes a negative integer", {
      amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output =
          amanuensis::Writer::WriteToString(amanuensis::JsonValue{-99LL}, minified_options);
      ASSERT_EQUAL(output, std::string("-99"));
    });

    IT("writes zero", {
      amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output =
          amanuensis::Writer::WriteToString(amanuensis::JsonValue{0LL}, minified_options);
      ASSERT_EQUAL(output, std::string("0"));
    });
  });

  DESCRIBE("Double output", {
    IT("writes a double with a decimal point", {
      amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output =
          amanuensis::Writer::WriteToString(amanuensis::JsonValue{3.14}, minified_options);
      ASSERT_TRUE(output.find('.') != std::string::npos || output.find('e') != std::string::npos);
    });

    IT("round-trips a double through write then parse", {
      double original_value = 0.1 + 0.2;
      amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string text = amanuensis::Writer::WriteToString(
          amanuensis::JsonValue{original_value}, minified_options
      );

      auto parse_result = amanuensis::Reader::ParseString(text);
      ASSERT_TRUE(parse_result.succeeded);
      ASSERT_TRUE(amanuensis::Json::IsDouble(parse_result.value));
      ASSERT_TRUE(amanuensis::Json::AsDouble(parse_result.value) == original_value);
    });

    IT("writes 1.0 with a decimal point", {
      amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output =
          amanuensis::Writer::WriteToString(amanuensis::JsonValue{1.0}, minified_options);
      ASSERT_TRUE(output.find('.') != std::string::npos || output.find('e') != std::string::npos);
    });
  });

  DESCRIBE("String output", {
    IT("writes a simple string with quotes", {
      amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output = amanuensis::Writer::WriteToString(
          amanuensis::JsonValue{std::string("hello")}, minified_options
      );
      ASSERT_EQUAL(output, std::string("\"hello\""));
    });

    IT("escapes special characters in strings", {
      amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output = amanuensis::Writer::WriteToString(
          amanuensis::JsonValue{std::string("a\nb\\c\"d")}, minified_options
      );
      ASSERT_EQUAL(output, std::string("\"a\\nb\\\\c\\\"d\""));
    });

    IT("escapes control characters as \\u00XX", {
      amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string input(1, '\x01');
      std::string output =
          amanuensis::Writer::WriteToString(amanuensis::JsonValue{input}, minified_options);
      ASSERT_EQUAL(output, std::string("\"\\u0001\""));
    });
  });

  DESCRIBE("Pretty vs minified", {
    IT("pretty-prints an object with newlines and indentation", {
      amanuensis::JsonValue object_value = amanuensis::Json::MakeObject();
      amanuensis::Json::Insert(object_value, "name", amanuensis::JsonValue{std::string("Alice")});
      amanuensis::Json::Insert(object_value, "age", amanuensis::JsonValue{30LL});

      std::string pretty_output = amanuensis::Writer::WriteToString(object_value);
      ASSERT_TRUE(pretty_output.find('\n') != std::string::npos);
      ASSERT_TRUE(pretty_output.find("  ") != std::string::npos);
      ASSERT_TRUE(pretty_output.find("\"name\": \"Alice\"") != std::string::npos);
    });

    IT("minifies an object without whitespace", {
      amanuensis::JsonValue object_value = amanuensis::Json::MakeObject();
      amanuensis::Json::Insert(object_value, "a", amanuensis::JsonValue{1LL});

      amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;

      std::string output = amanuensis::Writer::WriteToString(object_value, minified_options);
      ASSERT_EQUAL(output, std::string("{\"a\":1}"));
    });

    IT("appends trailing newline by default", {
      std::string output = amanuensis::Writer::WriteToString(amanuensis::JsonValue{42LL});
      ASSERT_TRUE(!output.empty());
      ASSERT_EQUAL(output.back(), '\n');
    });

    IT("omits trailing newline when configured", {
      amanuensis::WriterOptions no_newline_options;
      no_newline_options.trailingNewline = false;
      std::string output =
          amanuensis::Writer::WriteToString(amanuensis::JsonValue{42LL}, no_newline_options);
      ASSERT_TRUE(output.back() != '\n');
    });
  });

  DESCRIBE("Empty containers", {
    IT("writes empty array as []", {
      amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output =
          amanuensis::Writer::WriteToString(amanuensis::Json::MakeArray(), minified_options);
      ASSERT_EQUAL(output, std::string("[]"));
    });

    IT("writes empty object as {}", {
      amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output =
          amanuensis::Writer::WriteToString(amanuensis::Json::MakeObject(), minified_options);
      ASSERT_EQUAL(output, std::string("{}"));
    });
  });

  DESCRIBE("File output", {
    IT("writes to a file and returns true on success", {
      amanuensis::JsonValue object_value = amanuensis::Json::MakeObject();
      amanuensis::Json::Insert(object_value, "test", amanuensis::JsonValue{true});
      bool result =
          amanuensis::Writer::WriteToFile(object_value, "/tmp/amanuensis_writer_test.json");
      ASSERT_TRUE(result);
    });

    IT("returns false for an invalid file path", {
      amanuensis::JsonValue object_value = amanuensis::Json::MakeObject();
      bool result =
          amanuensis::Writer::WriteToFile(object_value, "/nonexistent_directory/file.json");
      ASSERT_FALSE(result);
    });
  });
});
