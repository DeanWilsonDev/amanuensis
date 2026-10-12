#include <cimmerian/test.hpp>
#include "support/calamus.hpp"

#include <string>

DESCRIBE("Calamus conformance", {
  DESCRIBE("Hole 9: empty objects", {
    IT("writes and reads an empty object as a bare block line", {
      core::Value entity = Json(R"({"light":{},"tags":[],"nested":{"inner":{}}})");
      std::string written = Write(entity, Json("{}"));
      ASSERT_TRUE(
          written.find("* light:\n* tags : []\n* nested:\n*   inner:\n") != std::string::npos
      );
      ASSERT_EQUAL(Minified(Load(written, Json("{}")).value), Minified(entity));
    });
  });
});
