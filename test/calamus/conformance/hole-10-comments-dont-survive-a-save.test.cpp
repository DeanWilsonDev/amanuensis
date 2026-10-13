#include <cimmerian/test.hpp>
#include "conformance-support.hpp"

#include <string>

using namespace conformance_test;

DESCRIBE("Calamus conformance", {
  DESCRIBE("Hole 10: comments don't survive a save", {
    IT("rejects a comment rather than dropping it", {
      std::string edited =
          Replace(Write(Door, Prefab), "  name : \"door\"", "  name : \"door\"  # front");
      auto read = calamus::Reader::ParseString(edited);
      ASSERT_FALSE(read.succeeded);
      ASSERT_EQUAL(read.error.message, std::string("Comments aren't supported in Calamus v1"));
    });
  });
});
