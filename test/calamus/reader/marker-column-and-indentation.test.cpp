#include <cimmerian/test.hpp>
#include "reader-support.hpp"

#include <string>

using namespace calamus_reader_test;

DESCRIBE("Calamus reader", {
  DESCRIBE("Marker column and indentation", {
    IT("rejects anything but '*' or a space in the marker column", {
      ASSERT_EQUAL(
          BodyErrorOf("position:\n"),
          std::string("4:1 Expected '*' or a space in the marker column")
      );
    });

    IT("needs a space after the marker", {
      ASSERT_EQUAL(
          BodyErrorOf("*position:\n"), std::string("4:2 Expected a space after the marker column")
      );
      ASSERT_EQUAL(BodyErrorOf("*\n"), std::string("4:2 Expected a field after the marker"));
    });

    IT("rejects tabs in indentation", {
      ASSERT_EQUAL(BodyErrorOf("\tx : 1\n"), std::string("4:1 Tabs aren't allowed in indentation"));
      ASSERT_EQUAL(
          BodyErrorOf("  a:\n  \tx : 1\n"), std::string("5:3 Tabs aren't allowed in indentation")
      );
    });

    IT("rejects an odd number of spaces", {
      ASSERT_EQUAL(
          BodyErrorOf("  a:\n     x : 1\n"),
          std::string("5:6 Indentation must be a multiple of two spaces")
      );
    });

    IT("rejects a line more than one level deeper than its parent", {
      ASSERT_EQUAL(
          BodyErrorOf("  a:\n      x : 1\n"),
          std::string("5:7 Indented more than one level under the line above")
      );
    });

    IT("rejects lines indented under a line that isn't a block", {
      ASSERT_EQUAL(
          BodyErrorOf("  a : 1\n    x : 1\n"),
          std::string("5:5 Only a 'key:' or '-' block line can have lines indented under it")
      );
      ASSERT_EQUAL(
          BodyErrorOf("  a:\n    - 1\n      x : 1\n"),
          std::string("6:7 Only a 'key:' or '-' block line can have lines indented under it")
      );
    });

    IT("rejects an indented first body line", {
      ASSERT_EQUAL(
          BodyErrorOf("    x : 1\n"),
          std::string("4:5 The first line of the body can't be indented")
      );
    });

    IT("closes a block when the indentation drops back", {
      ASSERT_EQUAL(
          BodyOf("  a:\n    b:\n      c : 1\n    d : 2\n  e : 3\n"),
          std::string(R"({"a":{"b":{"c":1},"d":2},"e":3})")
      );
    });
  });
});
