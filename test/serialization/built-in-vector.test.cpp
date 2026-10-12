#include <cimmerian/test.hpp>
#include "serialization-support.hpp"

#include <vector>

using namespace serialization_test;

DESCRIBE("Serialisation", {
  DESCRIBE("Built-in SerialTraits: std::vector", {
    IT("serialises a vector of ints", {
      auto original = MakeIntVector();
      amanuensis::core::Value json_value = amanuensis::ToValue(original);
      ASSERT_TRUE(json_value.IsArray());
      ASSERT_EQUAL(json_value.Size(), 3u);
      ASSERT_EQUAL(json_value.At(0).AsInteger(), 10LL);
      ASSERT_EQUAL(json_value.At(2).AsInteger(), 30LL);
    });

    IT("deserialises a vector of ints", {
      auto original = MakeIntVector();
      amanuensis::core::Value json_value = amanuensis::ToValue(original);
      auto round_tripped = amanuensis::FromValue<std::vector<int>>(json_value);
      ASSERT_EQUAL(round_tripped.size(), 3u);
      ASSERT_EQUAL(round_tripped[0], 10);
      ASSERT_EQUAL(round_tripped[2], 30);
    });

    IT("handles an empty vector", {
      std::vector<int> empty_vector;
      amanuensis::core::Value json_value = amanuensis::ToValue(empty_vector);
      ASSERT_TRUE(json_value.IsArray());
      ASSERT_EQUAL(json_value.Size(), 0u);
    });

    IT("serialises a vector of user types", {
      auto vectors = MakeVec3Vector();
      amanuensis::core::Value json_value = amanuensis::ToValue(vectors);
      ASSERT_EQUAL(json_value.Size(), 2u);
      ASSERT_EQUAL(json_value.At(0).Size(), 3u);
    });
  });
});
