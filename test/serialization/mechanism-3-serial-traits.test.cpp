#include <cimmerian/test.hpp>
#include "serialization-support.hpp"

#include <cmath>

using namespace serialization_test;

DESCRIBE("Serialisation", {
  DESCRIBE("Mechanism 3: SerialTraits specialisation", {
    IT("serialises to an array", {
      auto original = MakeSimpleVec3();
      amanuensis::core::Value json_value = amanuensis::ToValue(original);
      ASSERT_TRUE(json_value.IsArray());
      ASSERT_EQUAL(json_value.Size(), 3u);
    });

    IT("round-trips through ToValue and FromValue", {
      auto original = MakeVec3();
      amanuensis::core::Value json_value = amanuensis::ToValue(original);
      Vec3 round_tripped = amanuensis::FromValue<Vec3>(json_value);

      ASSERT_TRUE(std::abs(round_tripped.x - 1.5) < 1e-15);
      ASSERT_TRUE(std::abs(round_tripped.y - (-2.5)) < 1e-15);
      ASSERT_TRUE(std::abs(round_tripped.z - 3.0) < 1e-15);
    });
  });
});
