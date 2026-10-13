#include <cimmerian/test.hpp>
#include "serialization-support.hpp"
#include <amanuensis/json/reader.hpp>
#include <amanuensis/json/writer.hpp>

#include <string>

using namespace serialization_test;

DESCRIBE("Serialisation", {
  DESCRIBE("Full serialisation round-trip through file", {
    IT("writes and reads back a struct through JSON file", {
      auto original = MakeTestCoverage2();
      amanuensis::core::Value json_value = amanuensis::ToValue(original);

      bool write_succeeded =
          amanuensis::json::Writer::WriteToFile(json_value, "/tmp/amanuensis_serial_test.json");
      REQUIRE_TRUE(write_succeeded);

      auto parse_result = amanuensis::json::Reader::ParseFile("/tmp/amanuensis_serial_test.json");
      REQUIRE_TRUE(parse_result.succeeded);

      PerFunctionCoverage round_tripped =
          amanuensis::FromValue<PerFunctionCoverage>(parse_result.value);
      ASSERT_EQUAL(round_tripped.qualifiedName, std::string("math::Multiply"));
      ASSERT_EQUAL(round_tripped.startLine, 20);
      ASSERT_EQUAL(round_tripped.linesCovered, 8);
      ASSERT_EQUAL(round_tripped.executionCount, 7);
    });
  });
});
