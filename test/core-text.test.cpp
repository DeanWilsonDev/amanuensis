#include <cimmerian/test.hpp>
#include "core/cursor.hpp"
#include "core/number-format.hpp"
#include "core/quoted-string/quoted-string.hpp"
#include <amanuensis/core/value.hpp>

#include <bit>
#include <charconv>
#include <climits>
#include <cstdint>
#include <string>
#include <string_view>

namespace core = amanuensis::core;

static std::string FormatDouble(double value)
{
  std::string output;
  core::AppendDouble(output, value);
  return output;
}

static std::string FormatInteger(long long value)
{
  std::string output;
  core::AppendInteger(output, value);
  return output;
}

static bool ReadsBackIdentical(double value)
{
  std::string text = FormatDouble(value);
  double parsed = 0.0;
  auto [endPointer, errorCode] = std::from_chars(text.data(), text.data() + text.size(), parsed);
  return errorCode == std::errc() &&
         std::bit_cast<std::uint64_t>(parsed) == std::bit_cast<std::uint64_t>(value);
}

DESCRIBE("Shared text helpers", {
  DESCRIBE("Cursor", {
    IT("starts at line 1, column 1", {
      core::Cursor cursor("ab");
      ASSERT_EQUAL(cursor.Line(), 1);
      ASSERT_EQUAL(cursor.Column(), 1);
    });

    IT("moves to the next line after a newline", {
      core::Cursor cursor("ab\ncd");
      cursor.Advance();
      cursor.Advance();
      ASSERT_EQUAL(cursor.Column(), 3);
      cursor.Advance();
      ASSERT_EQUAL(cursor.Line(), 2);
      ASSERT_EQUAL(cursor.Column(), 1);
      cursor.Advance();
      ASSERT_EQUAL(cursor.Column(), 2);
    });

    IT("peeks '\\0' once the input is used up", {
      core::Cursor cursor("x");
      ASSERT_EQUAL(cursor.Advance(), 'x');
      ASSERT_TRUE(cursor.IsAtEnd());
      ASSERT_EQUAL(cursor.Peek(), '\0');
    });

    IT("slices the text consumed since a position", {
      core::Cursor cursor("key : value");
      std::size_t start = cursor.Position();
      cursor.Advance();
      cursor.Advance();
      cursor.Advance();
      ASSERT_EQUAL(cursor.Slice(start), std::string_view("key"));
    });

    IT("makes an error at the current line and column", {
      core::Cursor cursor("a\nbc");
      cursor.Advance();
      cursor.Advance();
      cursor.Advance();
      auto result = cursor.MakeError("boom");
      ASSERT_FALSE(result.succeeded);
      ASSERT_EQUAL(result.error.message, std::string("boom"));
      ASSERT_EQUAL(result.error.line, 2);
      ASSERT_EQUAL(result.error.column, 2);
    });
  });

  DESCRIBE("Quoted strings", {
    IT("reads escapes and leaves the cursor after the closing quote", {
      core::Cursor cursor(R"("a\"b\\c\n\/" rest)");
      auto result = core::ParseQuotedString(cursor);
      REQUIRE_TRUE(result.succeeded);
      ASSERT_EQUAL(result.value.AsString(), std::string("a\"b\\c\n/"));
      ASSERT_EQUAL(cursor.Peek(), ' ');
    });

    IT("decodes a surrogate pair to UTF-8", {
      core::Cursor cursor(R"("\uD83D\uDE00")");
      auto result = core::ParseQuotedString(cursor);
      REQUIRE_TRUE(result.succeeded);
      ASSERT_EQUAL(result.value.AsString(), std::string("\xF0\x9F\x98\x80"));
    });

    IT("reports an unterminated string at the end of the input", {
      core::Cursor cursor("\"abc");
      auto result = core::ParseQuotedString(cursor);
      ASSERT_FALSE(result.succeeded);
      ASSERT_EQUAL(result.error.message, std::string("Unterminated string literal"));
      ASSERT_EQUAL(result.error.line, 1);
      ASSERT_EQUAL(result.error.column, 5);
    });

    IT("reports a raw control character at its line and column", {
      core::Cursor cursor("\n\"ab\tc\"");
      cursor.Advance();
      auto result = core::ParseQuotedString(cursor);
      ASSERT_FALSE(result.succeeded);
      ASSERT_EQUAL(result.error.message, std::string("Unescaped control character in string"));
      ASSERT_EQUAL(result.error.line, 2);
      ASSERT_EQUAL(result.error.column, 4);
    });

    IT("writes control characters as \\u00XX", {
      std::string output;
      core::AppendQuotedString(output, std::string("\x01"));
      ASSERT_EQUAL(output, std::string("\"\\u0001\""));
    });

    IT("reads back what it writes", {
      std::string original =
          "quote \" backslash \\ newline \n tab \t bell \x07 caf\xC3\xA9 , : # |";
      std::string written;
      core::AppendQuotedString(written, original);

      core::Cursor cursor(written);
      auto result = core::ParseQuotedString(cursor);
      REQUIRE_TRUE(result.succeeded);
      ASSERT_EQUAL(result.value.AsString(), original);
      ASSERT_TRUE(cursor.IsAtEnd());
    });
  });

  DESCRIBE("Number reading", {
    IT("reads digits alone as an Integer and leaves the cursor after them", {
      core::Cursor cursor("-42 rest");
      auto result = core::ParseNumber(cursor);
      REQUIRE_TRUE(result.succeeded);
      ASSERT_TRUE(result.value.IsInteger());
      ASSERT_EQUAL(result.value.AsInteger(), -42LL);
      ASSERT_EQUAL(cursor.Peek(), ' ');
    });

    IT("reads a '.' or an exponent as a Double", {
      core::Cursor fraction("1.0");
      auto fractionResult = core::ParseNumber(fraction);
      REQUIRE_TRUE(fractionResult.succeeded);
      ASSERT_TRUE(fractionResult.value.IsDouble());
      ASSERT_EQUAL(fractionResult.value.AsDouble(), 1.0);

      core::Cursor exponent("2e3");
      auto exponentResult = core::ParseNumber(exponent);
      REQUIRE_TRUE(exponentResult.succeeded);
      ASSERT_TRUE(exponentResult.value.IsDouble());
      ASSERT_EQUAL(exponentResult.value.AsDouble(), 2000.0);
    });

    IT("reads an integer that overflows long long as a Double", {
      core::Cursor cursor("9223372036854775808");
      auto result = core::ParseNumber(cursor);
      REQUIRE_TRUE(result.succeeded);
      ASSERT_TRUE(result.value.IsDouble());
      ASSERT_EQUAL(result.value.AsDouble(), 9223372036854775808.0);
    });

    IT("rejects a leading zero at its line and column", {
      core::Cursor cursor("\n012");
      cursor.Advance();
      auto result = core::ParseNumber(cursor);
      ASSERT_FALSE(result.succeeded);
      ASSERT_EQUAL(result.error.message, std::string("Leading zeros are not allowed in numbers"));
      ASSERT_EQUAL(result.error.line, 2);
      ASSERT_EQUAL(result.error.column, 2);
    });

    IT("rejects a '.' with no digits after it", {
      core::Cursor cursor("1.");
      auto result = core::ParseNumber(cursor);
      ASSERT_FALSE(result.succeeded);
      ASSERT_EQUAL(result.error.message, std::string("Expected digit after decimal point"));
      ASSERT_EQUAL(result.error.column, 3);
    });

    IT("reads back what it writes", {
      std::string written = FormatDouble(0.1);
      core::Cursor cursor(written);
      auto result = core::ParseNumber(cursor);
      REQUIRE_TRUE(result.succeeded);
      ASSERT_EQUAL(
          std::bit_cast<std::uint64_t>(result.value.AsDouble()), std::bit_cast<std::uint64_t>(0.1)
      );
      ASSERT_TRUE(cursor.IsAtEnd());
    });
  });

  DESCRIBE("Number formatting", {
    IT("writes integers in plain decimal", {
      ASSERT_EQUAL(FormatInteger(0), std::string("0"));
      ASSERT_EQUAL(FormatInteger(-42), std::string("-42"));
      ASSERT_EQUAL(FormatInteger(LLONG_MIN), std::string("-9223372036854775808"));
    });

    IT("adds .0 to a whole double", {
      ASSERT_EQUAL(FormatDouble(1.0), std::string("1.0"));
      ASSERT_EQUAL(FormatDouble(-3.0), std::string("-3.0"));
      ASSERT_EQUAL(FormatDouble(100.0), std::string("100.0"));
    });

    IT("writes the shortest form that reads back", {
      ASSERT_EQUAL(FormatDouble(0.1), std::string("0.1"));
      ASSERT_EQUAL(FormatDouble(46.103531), std::string("46.103531"));
    });

    IT("leaves exponent forms without .0", {
      ASSERT_EQUAL(FormatDouble(1e21), std::string("1e+21"));
      ASSERT_EQUAL(FormatDouble(1e-7), std::string("1e-07"));
    });

    IT("writes a float in the shortest form that reads back as that float", {
      std::string output;
      core::AppendFloat(output, 0.1f);
      ASSERT_EQUAL(output, std::string("0.1"));
      output.clear();
      core::AppendFloat(output, 2.0f);
      ASSERT_EQUAL(output, std::string("2.0"));
      output.clear();
      core::AppendFloat(output, 1e20f);
      ASSERT_EQUAL(output, std::string("1e+20"));
    });

    IT("reads back bit-identical", {
      ASSERT_TRUE(ReadsBackIdentical(0.1));
      ASSERT_TRUE(ReadsBackIdentical(-0.0));
      ASSERT_TRUE(ReadsBackIdentical(1.0 / 3.0));
      ASSERT_TRUE(ReadsBackIdentical(5e-324));
      ASSERT_TRUE(ReadsBackIdentical(1.7976931348623157e308));
      ASSERT_TRUE(ReadsBackIdentical(static_cast<double>(0.1f)));
    });
  });
});
