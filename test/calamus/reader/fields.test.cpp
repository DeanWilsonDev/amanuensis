#include <cimmerian/test.hpp>
#include "reader-support.hpp"

#include <string>

using namespace calamus_reader_test;

DESCRIBE("Calamus reader", {
  DESCRIBE("Fields", {
    IT("honours marks on field lines",
       { ASSERT_EQUAL(FieldsOf("* x : 1\n  y : 2\n"), std::string("x* y")); });

    IT("treats a block line as a field when its block is an array or an empty object", {
      ASSERT_EQUAL(
          FieldsOf("* list:\n    - 1\n* empty:\n  object:\n*   z : 1\n"),
          std::string("list* empty* object.z*")
      );
    });

    IT("ignores the mark on a block line that opens a non-empty object",
       { ASSERT_EQUAL(FieldsOf("* a:\n    x : 1\n"), std::string("a.x")); });

    IT("never lists lines inside an array as fields", {
      ASSERT_EQUAL(
          FieldsOf("  list:\n    -\n*     x : 1\n      inner:\n        y : 2\n"),
          std::string("list")
      );
    });

    IT("lists an inline array as one field",
       { ASSERT_EQUAL(FieldsOf("* tags : [\"a\"]\n"), std::string("tags*")); });
  });
});
