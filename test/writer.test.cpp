#include "cimmerian/test.hpp"
#include <amanuensis/core/value.hpp>
#include <amanuensis/json.hpp>
#include <amanuensis/core/parse-result.hpp>
#include <amanuensis/json/writer.hpp>
#include <amanuensis/json/reader.hpp>

#include <string>
#include <variant>

DESCRIBE("Writer", {
  DESCRIBE("Null output", {
    IT("writes null", {
      amanuensis::core::Value null_value{std::monostate()};
      amanuensis::json::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output = amanuensis::json::Writer::WriteToString(null_value, minified_options);
      ASSERT_EQUAL(output, std::string("null"));
    });
  });

  DESCRIBE("Boolean output", {
    IT("writes true", {
      amanuensis::json::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output =
          amanuensis::json::Writer::WriteToString(amanuensis::core::Value{true}, minified_options);
      ASSERT_EQUAL(output, std::string("true"));
    });

    IT("writes false", {
      amanuensis::json::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output =
          amanuensis::json::Writer::WriteToString(amanuensis::core::Value{false}, minified_options);
      ASSERT_EQUAL(output, std::string("false"));
    });
  });

  DESCRIBE("Integer output", {
    IT("writes a positive integer without decimal point", {
      amanuensis::json::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output =
          amanuensis::json::Writer::WriteToString(amanuensis::core::Value{42LL}, minified_options);
      ASSERT_EQUAL(output, std::string("42"));
    });

    IT("writes a negative integer", {
      amanuensis::json::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output =
          amanuensis::json::Writer::WriteToString(amanuensis::core::Value{-99LL}, minified_options);
      ASSERT_EQUAL(output, std::string("-99"));
    });

    IT("writes zero", {
      amanuensis::json::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output =
          amanuensis::json::Writer::WriteToString(amanuensis::core::Value{0LL}, minified_options);
      ASSERT_EQUAL(output, std::string("0"));
    });
  });

  DESCRIBE("Double output", {
    IT("writes a double with a decimal point", {
      amanuensis::json::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output =
          amanuensis::json::Writer::WriteToString(amanuensis::core::Value{3.14}, minified_options);
      ASSERT_TRUE(output.find('.') != std::string::npos || output.find('e') != std::string::npos);
    });

    IT("round-trips a double through write then parse", {
      double original_value = 0.1 + 0.2;
      amanuensis::json::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string text = amanuensis::json::Writer::WriteToString(
          amanuensis::core::Value{original_value}, minified_options
      );

      auto parse_result = amanuensis::json::Reader::ParseString(text);
      ASSERT_TRUE(parse_result.succeeded);
      ASSERT_TRUE(amanuensis::Json::IsDouble(parse_result.value));
      ASSERT_TRUE(amanuensis::Json::AsDouble(parse_result.value) == original_value);
    });

    IT("writes 1.0 with a decimal point", {
      amanuensis::json::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output =
          amanuensis::json::Writer::WriteToString(amanuensis::core::Value{1.0}, minified_options);
      ASSERT_TRUE(output.find('.') != std::string::npos || output.find('e') != std::string::npos);
    });
  });

  DESCRIBE("String output", {
    IT("writes a simple string with quotes", {
      amanuensis::json::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output = amanuensis::json::Writer::WriteToString(
          amanuensis::core::Value{std::string("hello")}, minified_options
      );
      ASSERT_EQUAL(output, std::string("\"hello\""));
    });

    IT("escapes special characters in strings", {
      amanuensis::json::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output = amanuensis::json::Writer::WriteToString(
          amanuensis::core::Value{std::string("a\nb\\c\"d")}, minified_options
      );
      ASSERT_EQUAL(output, std::string("\"a\\nb\\\\c\\\"d\""));
    });

    IT("escapes control characters as \\u00XX", {
      amanuensis::json::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string input(1, '\x01');
      std::string output =
          amanuensis::json::Writer::WriteToString(amanuensis::core::Value{input}, minified_options);
      ASSERT_EQUAL(output, std::string("\"\\u0001\""));
    });
  });

  DESCRIBE("Pretty vs minified", {
    IT("pretty-prints an object with newlines and indentation", {
      amanuensis::core::Value object_value = amanuensis::Json::MakeObject();
      amanuensis::Json::Insert(object_value, "name", amanuensis::core::Value{std::string("Alice")});
      amanuensis::Json::Insert(object_value, "age", amanuensis::core::Value{30LL});

      std::string pretty_output = amanuensis::json::Writer::WriteToString(object_value);
      ASSERT_TRUE(pretty_output.find('\n') != std::string::npos);
      ASSERT_TRUE(pretty_output.find("  ") != std::string::npos);
      ASSERT_TRUE(pretty_output.find("\"name\": \"Alice\"") != std::string::npos);
    });

    IT("minifies an object without whitespace", {
      amanuensis::core::Value object_value = amanuensis::Json::MakeObject();
      amanuensis::Json::Insert(object_value, "a", amanuensis::core::Value{1LL});

      amanuensis::json::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;

      std::string output = amanuensis::json::Writer::WriteToString(object_value, minified_options);
      ASSERT_EQUAL(output, std::string("{\"a\":1}"));
    });

    IT("appends trailing newline by default", {
      std::string output = amanuensis::json::Writer::WriteToString(amanuensis::core::Value{42LL});
      ASSERT_TRUE(!output.empty());
      ASSERT_EQUAL(output.back(), '\n');
    });

    IT("omits trailing newline when configured", {
      amanuensis::json::WriterOptions no_newline_options;
      no_newline_options.trailingNewline = false;
      std::string output = amanuensis::json::Writer::WriteToString(
          amanuensis::core::Value{42LL}, no_newline_options
      );
      ASSERT_TRUE(output.back() != '\n');
    });
  });

  DESCRIBE("Empty containers", {
    IT("writes empty array as []", {
      amanuensis::json::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output =
          amanuensis::json::Writer::WriteToString(amanuensis::Json::MakeArray(), minified_options);
      ASSERT_EQUAL(output, std::string("[]"));
    });

    IT("writes empty object as {}", {
      amanuensis::json::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output =
          amanuensis::json::Writer::WriteToString(amanuensis::Json::MakeObject(), minified_options);
      ASSERT_EQUAL(output, std::string("{}"));
    });
  });

  DESCRIBE("File output", {
    IT("writes to a file and returns true on success", {
      amanuensis::core::Value object_value = amanuensis::Json::MakeObject();
      amanuensis::Json::Insert(object_value, "test", amanuensis::core::Value{true});
      bool result =
          amanuensis::json::Writer::WriteToFile(object_value, "/tmp/amanuensis_writer_test.json");
      ASSERT_TRUE(result);
    });

    IT("returns false for an invalid file path", {
      amanuensis::core::Value object_value = amanuensis::Json::MakeObject();
      bool result =
          amanuensis::json::Writer::WriteToFile(object_value, "/nonexistent_directory/file.json");
      ASSERT_FALSE(result);
    });
  });
});
