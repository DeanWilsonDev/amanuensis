#include <cimmerian/test.hpp>
#include "serialization-support.hpp"
#include <amanuensis/json/reader.hpp>

#include <string>

using namespace serialization_test;

DESCRIBE("Serialisation", {
  DESCRIBE("Extra fields in JSON", {
    IT("silently ignores extra fields during deserialisation", {
      auto parsed = amanuensis::json::Reader::ParseString(
          R"({"display_name":"W","item_count":5,"unknown_field":"ignored"})"
      );
      REQUIRE_TRUE(parsed.succeeded);
      auto result = amanuensis::TryFromValue<RenamedFields>(parsed.value);
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(result.value.name, std::string("W"));
      ASSERT_EQUAL(result.value.count, 5);
    });
  });
});
