#include <cimmerian/test.hpp>
#include "support/calamus.hpp"

#include <string>

DESCRIBE("Calamus conformance", {
  DESCRIBE("Hole 2: a one-item list looks like a scalar", {
    IT("always brackets arrays", {
      core::Value entity = Json(R"({"tags":["a"],"name":"a"})");
      std::string written = Write(entity, Json("{}"));
      ASSERT_TRUE(written.find("* tags : [\"a\"]\n") != std::string::npos);
      ASSERT_EQUAL(Minified(Load(written, Json("{}")).value), Minified(entity));
    });
  });
});
