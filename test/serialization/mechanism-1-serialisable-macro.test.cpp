#include <cimmerian/test.hpp>
#include "serialization-support.hpp"

#include <string>

using namespace serialization_test;

DESCRIBE("Serialisation", {
  DESCRIBE("Mechanism 1: AMANUENSIS_SERIALISABLE macro", {
    IT("serialises a struct to JSON", {
      auto original = MakeTestCoverage();
      amanuensis::core::Value json_value = amanuensis::ToValue(original);

      ASSERT_TRUE(json_value.IsObject());
      ASSERT_EQUAL(json_value.Get("qualifiedName").AsString(), std::string("math::Add"));
      ASSERT_EQUAL(json_value.Get("startLine").AsInteger(), 10LL);
      ASSERT_EQUAL(json_value.Get("endLine").AsInteger(), 14LL);
      ASSERT_EQUAL(json_value.Get("linesTotal").AsInteger(), 5LL);
      ASSERT_EQUAL(json_value.Get("linesCovered").AsInteger(), 5LL);
      ASSERT_EQUAL(json_value.Get("executionCount").AsInteger(), 3LL);
    });

    IT("deserialises JSON back to a struct", {
      auto original = MakeTestCoverage();
      amanuensis::core::Value json_value = amanuensis::ToValue(original);
      PerFunctionCoverage round_tripped = amanuensis::FromValue<PerFunctionCoverage>(json_value);

      ASSERT_EQUAL(round_tripped.qualifiedName, std::string("math::Add"));
      ASSERT_EQUAL(round_tripped.startLine, 10);
      ASSERT_EQUAL(round_tripped.endLine, 14);
      ASSERT_EQUAL(round_tripped.linesTotal, 5);
      ASSERT_EQUAL(round_tripped.linesCovered, 5);
      ASSERT_EQUAL(round_tripped.executionCount, 3);
    });

    IT("uses C++ field names as JSON keys", {
      auto original = MakeMinimalCoverage();
      amanuensis::core::Value json_value = amanuensis::ToValue(original);
      ASSERT_TRUE(json_value.Contains("qualifiedName"));
      ASSERT_TRUE(json_value.Contains("startLine"));
      ASSERT_TRUE(json_value.Contains("endLine"));
    });
  });
});
