#include "cimmerian/test.hpp"
#include "amanuensis/writer.hpp"
#include "amanuensis/reader.hpp"

#include <cmath>
#include <string>

DESCRIBE("Writer", {

  DESCRIBE("Null output", {
    IT("writes null", {
      Amanuensis::Value nullValue;
      Amanuensis::WriterOptions minifiedOptions;
      minifiedOptions.pretty = false;
      minifiedOptions.trailingNewline = false;
      std::string output = Amanuensis::Writer::WriteToString(nullValue, minifiedOptions);
      ASSERT_EQUAL(output, std::string("null"));
    });
  });

  DESCRIBE("Boolean output", {
    IT("writes true", {
      Amanuensis::WriterOptions minifiedOptions;
      minifiedOptions.pretty = false;
      minifiedOptions.trailingNewline = false;
      std::string output = Amanuensis::Writer::WriteToString(Amanuensis::Value(true), minifiedOptions);
      ASSERT_EQUAL(output, std::string("true"));
    });

    IT("writes false", {
      Amanuensis::WriterOptions minifiedOptions;
      minifiedOptions.pretty = false;
      minifiedOptions.trailingNewline = false;
      std::string output = Amanuensis::Writer::WriteToString(Amanuensis::Value(false), minifiedOptions);
      ASSERT_EQUAL(output, std::string("false"));
    });
  });

  DESCRIBE("Integer output", {
    IT("writes a positive integer without decimal point", {
      Amanuensis::WriterOptions minifiedOptions;
      minifiedOptions.pretty = false;
      minifiedOptions.trailingNewline = false;
      std::string output = Amanuensis::Writer::WriteToString(Amanuensis::Value(42), minifiedOptions);
      ASSERT_EQUAL(output, std::string("42"));
    });

    IT("writes a negative integer", {
      Amanuensis::WriterOptions minifiedOptions;
      minifiedOptions.pretty = false;
      minifiedOptions.trailingNewline = false;
      std::string output = Amanuensis::Writer::WriteToString(Amanuensis::Value(-99), minifiedOptions);
      ASSERT_EQUAL(output, std::string("-99"));
    });

    IT("writes zero", {
      Amanuensis::WriterOptions minifiedOptions;
      minifiedOptions.pretty = false;
      minifiedOptions.trailingNewline = false;
      std::string output = Amanuensis::Writer::WriteToString(Amanuensis::Value(0), minifiedOptions);
      ASSERT_EQUAL(output, std::string("0"));
    });
  });

  DESCRIBE("Double output", {
    IT("writes a double with a decimal point", {
      Amanuensis::WriterOptions minifiedOptions;
      minifiedOptions.pretty = false;
      minifiedOptions.trailingNewline = false;
      std::string output = Amanuensis::Writer::WriteToString(Amanuensis::Value(3.14), minifiedOptions);
      // Must contain a decimal point to distinguish from integer
      ASSERT_TRUE(output.find('.') != std::string::npos || output.find('e') != std::string::npos);
    });

    IT("round-trips a double through write then parse", {
      double originalValue = 0.1 + 0.2;
      Amanuensis::WriterOptions minifiedOptions;
      minifiedOptions.pretty = false;
      minifiedOptions.trailingNewline = false;
      std::string text = Amanuensis::Writer::WriteToString(Amanuensis::Value(originalValue), minifiedOptions);

      auto parseResult = Amanuensis::Reader::ParseString(text);
      ASSERT_TRUE(parseResult.succeeded);
      ASSERT_TRUE(parseResult.value.IsDouble());
      ASSERT_TRUE(parseResult.value.AsDouble() == originalValue);
    });

    IT("writes 1.0 with a decimal point", {
      Amanuensis::WriterOptions minifiedOptions;
      minifiedOptions.pretty = false;
      minifiedOptions.trailingNewline = false;
      std::string output = Amanuensis::Writer::WriteToString(Amanuensis::Value(1.0), minifiedOptions);
      // Must not emit just "1" — that would parse as integer
      ASSERT_TRUE(output.find('.') != std::string::npos || output.find('e') != std::string::npos);
    });
  });

  DESCRIBE("String output", {
    IT("writes a simple string with quotes", {
      Amanuensis::WriterOptions minifiedOptions;
      minifiedOptions.pretty = false;
      minifiedOptions.trailingNewline = false;
      std::string output = Amanuensis::Writer::WriteToString(Amanuensis::Value("hello"), minifiedOptions);
      ASSERT_EQUAL(output, std::string("\"hello\""));
    });

    IT("escapes special characters in strings", {
      Amanuensis::WriterOptions minifiedOptions;
      minifiedOptions.pretty = false;
      minifiedOptions.trailingNewline = false;
      std::string output =
          Amanuensis::Writer::WriteToString(Amanuensis::Value("a\nb\\c\"d"), minifiedOptions);
      ASSERT_EQUAL(output, std::string("\"a\\nb\\\\c\\\"d\""));
    });

    IT("escapes control characters as \\u00XX", {
      Amanuensis::WriterOptions minifiedOptions;
      minifiedOptions.pretty = false;
      minifiedOptions.trailingNewline = false;
      std::string input(1, '\x01');
      std::string output = Amanuensis::Writer::WriteToString(Amanuensis::Value(input), minifiedOptions);
      ASSERT_EQUAL(output, std::string("\"\\u0001\""));
    });
  });

  DESCRIBE("Pretty vs minified", {
    IT("pretty-prints an object with newlines and indentation", {
      Amanuensis::Value objectValue = Amanuensis::Value::MakeObject();
      objectValue.Insert("name", Amanuensis::Value("Alice"));
      objectValue.Insert("age", Amanuensis::Value(30));

      std::string prettyOutput = Amanuensis::Writer::WriteToString(objectValue);
      ASSERT_TRUE(prettyOutput.find('\n') != std::string::npos);
      ASSERT_TRUE(prettyOutput.find("  ") != std::string::npos);
      ASSERT_TRUE(prettyOutput.find("\"name\": \"Alice\"") != std::string::npos);
    });

    IT("minifies an object without whitespace", {
      Amanuensis::Value objectValue = Amanuensis::Value::MakeObject();
      objectValue.Insert("a", Amanuensis::Value(1));

      Amanuensis::WriterOptions minifiedOptions;
      minifiedOptions.pretty = false;
      minifiedOptions.trailingNewline = false;

      std::string output = Amanuensis::Writer::WriteToString(objectValue, minifiedOptions);
      ASSERT_EQUAL(output, std::string("{\"a\":1}"));
    });

    IT("appends trailing newline by default", {
      std::string output = Amanuensis::Writer::WriteToString(Amanuensis::Value(42));
      ASSERT_TRUE(!output.empty());
      ASSERT_EQUAL(output.back(), '\n');
    });

    IT("omits trailing newline when configured", {
      Amanuensis::WriterOptions noNewlineOptions;
      noNewlineOptions.trailingNewline = false;
      std::string output = Amanuensis::Writer::WriteToString(Amanuensis::Value(42), noNewlineOptions);
      ASSERT_TRUE(output.back() != '\n');
    });
  });

  DESCRIBE("Empty containers", {
    IT("writes empty array as []", {
      Amanuensis::WriterOptions minifiedOptions;
      minifiedOptions.pretty = false;
      minifiedOptions.trailingNewline = false;
      std::string output =
          Amanuensis::Writer::WriteToString(Amanuensis::Value::MakeArray(), minifiedOptions);
      ASSERT_EQUAL(output, std::string("[]"));
    });

    IT("writes empty object as {}", {
      Amanuensis::WriterOptions minifiedOptions;
      minifiedOptions.pretty = false;
      minifiedOptions.trailingNewline = false;
      std::string output =
          Amanuensis::Writer::WriteToString(Amanuensis::Value::MakeObject(), minifiedOptions);
      ASSERT_EQUAL(output, std::string("{}"));
    });
  });

  DESCRIBE("File output", {
    IT("writes to a file and returns true on success", {
      Amanuensis::Value objectValue = Amanuensis::Value::MakeObject();
      objectValue.Insert("test", Amanuensis::Value(true));
      bool result = Amanuensis::Writer::WriteToFile(objectValue, "/tmp/amanuensis_writer_test.json");
      ASSERT_TRUE(result);
    });

    IT("returns false for an invalid file path", {
      Amanuensis::Value objectValue = Amanuensis::Value::MakeObject();
      bool result = Amanuensis::Writer::WriteToFile(
          objectValue, "/nonexistent_directory/file.json"
      );
      ASSERT_FALSE(result);
    });
  });
});
