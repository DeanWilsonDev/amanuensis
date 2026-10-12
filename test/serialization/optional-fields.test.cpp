#include <cimmerian/test.hpp>
#include "serialization-support.hpp"
#include <amanuensis/json/reader.hpp>

#include <string>

using namespace serialization_test;

DESCRIBE("Serialisation", {
  DESCRIBE("Optional field handling in structs", {
    IT("populates optional field when present in JSON", {
      auto parsed = amanuensis::json::Reader::ParseString(
          R"({"key":"host","value":8080,"description":"Server port"})"
      );
      REQUIRE_TRUE(parsed.succeeded);
      auto entry = amanuensis::FromValue<ConfigEntry>(parsed.value);
      ASSERT_EQUAL(entry.key, std::string("host"));
      ASSERT_EQUAL(entry.value, 8080);
      ASSERT_TRUE(entry.description.has_value());
      ASSERT_EQUAL(*entry.description, std::string("Server port"));
    });

    IT("leaves optional field empty when absent in JSON", {
      auto parsed = amanuensis::json::Reader::ParseString(R"({"key":"host","value":8080})");
      REQUIRE_TRUE(parsed.succeeded);
      auto entry = amanuensis::FromValue<ConfigEntry>(parsed.value);
      ASSERT_EQUAL(entry.key, std::string("host"));
      ASSERT_FALSE(entry.description.has_value());
    });

    IT("leaves optional field empty when null in JSON", {
      auto parsed = amanuensis::json::Reader::ParseString(
          R"({"key":"host","value":8080,"description":null})"
      );
      REQUIRE_TRUE(parsed.succeeded);
      auto entry = amanuensis::FromValue<ConfigEntry>(parsed.value);
      ASSERT_FALSE(entry.description.has_value());
    });
  });
});
