#include <cimmerian/test.hpp>
#include "serialization-support.hpp"

#include <string>

using namespace serialization_test;

DESCRIBE("Serialisation", {
  DESCRIBE("TryFromValue non-throwing variant", {
    IT("succeeds on valid input", {
      auto original = MakeMinimalCoverage();
      amanuensis::core::Value json_value = amanuensis::ToValue(original);
      auto try_result = amanuensis::TryFromValue<PerFunctionCoverage>(json_value);
      ASSERT_TRUE(try_result.succeeded);
      ASSERT_EQUAL(try_result.value.qualifiedName, std::string("f"));
    });

    IT("fails on missing required fields without throwing", {
      amanuensis::core::Value empty_object = amanuensis::core::Value::MakeObject();
      auto try_result = amanuensis::TryFromValue<PerFunctionCoverage>(empty_object);
      ASSERT_FALSE(try_result.succeeded);
      ASSERT_FALSE(try_result.errorMessage.empty());
    });
  });
});
