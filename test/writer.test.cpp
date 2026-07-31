#include "cimmerian/test.hpp"
#include <amanuensis/value.hpp>
#include <amanuensis/json.hpp>
#include <amanuensis/io/parse-result.hpp>
#include <amanuensis/io/writer.hpp>
#include <amanuensis/io/reader.hpp>

#include <cmath>
#include <string>
#include <variant>

DESCRIBE("Writer", {
  DESCRIBE("Null output", {
    IT("writes null", {
      Amanuensis::Value null_value{std::monostate()};
      Amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output = Amanuensis::Writer::WriteToString(null_value, minified_options);
      ASSERT_EQUAL(output, std::string("null"));
    });
  });

  DESCRIBE("Boolean output", {
    IT("writes true", {
      Amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output =
          Amanuensis::Writer::WriteToString(Amanuensis::Value{true}, minified_options);
      ASSERT_EQUAL(output, std::string("true"));
    });

    IT("writes false", {
      Amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output =
          Amanuensis::Writer::WriteToString(Amanuensis::Value{false}, minified_options);
      ASSERT_EQUAL(output, std::string("false"));
    });
  });

  DESCRIBE("Integer output", {
    IT("writes a positive integer without decimal point", {
      Amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output =
          Amanuensis::Writer::WriteToString(Amanuensis::Value{42LL}, minified_options);
      ASSERT_EQUAL(output, std::string("42"));
    });

    IT("writes a negative integer", {
      Amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output =
          Amanuensis::Writer::WriteToString(Amanuensis::Value{-99LL}, minified_options);
      ASSERT_EQUAL(output, std::string("-99"));
    });

    IT("writes zero", {
      Amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output =
          Amanuensis::Writer::WriteToString(Amanuensis::Value{0LL}, minified_options);
      ASSERT_EQUAL(output, std::string("0"));
    });
  });

  DESCRIBE("Double output", {
    IT("writes a double with a decimal point", {
      Amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output =
          Amanuensis::Writer::WriteToString(Amanuensis::Value{3.14}, minified_options);
      ASSERT_TRUE(output.find('.') != std::string::npos || output.find('e') != std::string::npos);
    });

    IT("round-trips a double through write then parse", {
      double original_value = 0.1 + 0.2;
      Amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string text =
          Amanuensis::Writer::WriteToString(Amanuensis::Value{original_value}, minified_options);

      auto parse_result = Amanuensis::Reader::ParseString(text);
      ASSERT_TRUE(parse_result.succeeded);
      ASSERT_TRUE(Amanuensis::Json::IsDouble(parse_result.value));
      ASSERT_TRUE(Amanuensis::Json::AsDouble(parse_result.value) == original_value);
    });

    IT("writes 1.0 with a decimal point", {
      Amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output =
          Amanuensis::Writer::WriteToString(Amanuensis::Value{1.0}, minified_options);
      ASSERT_TRUE(output.find('.') != std::string::npos || output.find('e') != std::string::npos);
    });
  });

  DESCRIBE("String output", {
    IT("writes a simple string with quotes", {
      Amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output = Amanuensis::Writer::WriteToString(
          Amanuensis::Value{std::string("hello")}, minified_options
      );
      ASSERT_EQUAL(output, std::string("\"hello\""));
    });

    IT("escapes special characters in strings", {
      Amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output = Amanuensis::Writer::WriteToString(
          Amanuensis::Value{std::string("a\nb\\c\"d")}, minified_options
      );
      ASSERT_EQUAL(output, std::string("\"a\\nb\\\\c\\\"d\""));
    });

    IT("escapes control characters as \\u00XX", {
      Amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string input(1, '\x01');
      std::string output =
          Amanuensis::Writer::WriteToString(Amanuensis::Value{input}, minified_options);
      ASSERT_EQUAL(output, std::string("\"\\u0001\""));
    });
  });

  DESCRIBE("Pretty vs minified", {
    IT("pretty-prints an object with newlines and indentation", {
      Amanuensis::Value object_value = Amanuensis::Json::MakeObject();
      Amanuensis::Json::Insert(object_value, "name", Amanuensis::Value{std::string("Alice")});
      Amanuensis::Json::Insert(object_value, "age", Amanuensis::Value{30LL});

      std::string pretty_output = Amanuensis::Writer::WriteToString(object_value);
      ASSERT_TRUE(pretty_output.find('\n') != std::string::npos);
      ASSERT_TRUE(pretty_output.find("  ") != std::string::npos);
      ASSERT_TRUE(pretty_output.find("\"name\": \"Alice\"") != std::string::npos);
    });

    IT("minifies an object without whitespace", {
      Amanuensis::Value object_value = Amanuensis::Json::MakeObject();
      Amanuensis::Json::Insert(object_value, "a", Amanuensis::Value{1LL});

      Amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;

      std::string output = Amanuensis::Writer::WriteToString(object_value, minified_options);
      ASSERT_EQUAL(output, std::string("{\"a\":1}"));
    });

    IT("appends trailing newline by default", {
      std::string output = Amanuensis::Writer::WriteToString(Amanuensis::Value{42LL});
      ASSERT_TRUE(!output.empty());
      ASSERT_EQUAL(output.back(), '\n');
    });

    IT("omits trailing newline when configured", {
      Amanuensis::WriterOptions no_newline_options;
      no_newline_options.trailingNewline = false;
      std::string output =
          Amanuensis::Writer::WriteToString(Amanuensis::Value{42LL}, no_newline_options);
      ASSERT_TRUE(output.back() != '\n');
    });
  });

  DESCRIBE("Empty containers", {
    IT("writes empty array as []", {
      Amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output =
          Amanuensis::Writer::WriteToString(Amanuensis::Json::MakeArray(), minified_options);
      ASSERT_EQUAL(output, std::string("[]"));
    });

    IT("writes empty object as {}", {
      Amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string output =
          Amanuensis::Writer::WriteToString(Amanuensis::Json::MakeObject(), minified_options);
      ASSERT_EQUAL(output, std::string("{}"));
    });
  });

  DESCRIBE("File output", {
    IT("writes to a file and returns true on success", {
      Amanuensis::Value object_value = Amanuensis::Json::MakeObject();
      Amanuensis::Json::Insert(object_value, "test", Amanuensis::Value{true});
      bool result =
          Amanuensis::Writer::WriteToFile(object_value, "/tmp/amanuensis_writer_test.json");
      ASSERT_TRUE(result);
    });

    IT("returns false for an invalid file path", {
      Amanuensis::Value object_value = Amanuensis::Json::MakeObject();
      bool result =
          Amanuensis::Writer::WriteToFile(object_value, "/nonexistent_directory/file.json");
      ASSERT_FALSE(result);
    });
  });
});
