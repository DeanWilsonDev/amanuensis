#include <cimmerian/test.hpp>
#include "reader-support.hpp"

#include <string>

using namespace calamus_reader_test;

DESCRIBE("Calamus reader", {
  DESCRIBE("Comments", {
    IT("rejects '#' outside a string", {
      ASSERT_EQUAL(
          BodyErrorOf("  x : 1  # note\n"),
          std::string("4:10 Comments aren't supported in Calamus v1")
      );
      ASSERT_EQUAL(
          BodyErrorOf("  # note\n"), std::string("4:3 Comments aren't supported in Calamus v1")
      );
      ASSERT_EQUAL(
          BodyErrorOf("  x : # note\n"), std::string("4:7 Comments aren't supported in Calamus v1")
      );
    });
  });
});
