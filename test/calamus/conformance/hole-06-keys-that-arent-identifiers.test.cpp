#include <cimmerian/test.hpp>
#include "conformance-support.hpp"

#include <string>

using namespace conformance_test;

DESCRIBE("Calamus conformance", {
  DESCRIBE("Hole 6: keys that aren't identifiers", {
    IT("quotes them and reads them back", {
      core::Value entity = Json(R"({"door frame":1,"a:b":2,"x|y":3,"#":4,"\"q\"":5,"":6,"é":7})");
      std::string written = Write(entity, Json("{}"));
      ASSERT_EQUAL(Minified(Load(written, Json("{}")).value), Minified(entity));
      ASSERT_EQUAL(Heal(written, Json("{}")), written);
    });
  });
});
