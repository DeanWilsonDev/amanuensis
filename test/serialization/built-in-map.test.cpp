#include <cimmerian/test.hpp>
#include "serialization-support.hpp"

using namespace serialization_test;

DESCRIBE("Serialisation", {
  DESCRIBE("Built-in SerialTraits: std::map", {
    IT("serialises a string-keyed map", {
      auto original = MakeStringIntMap();
      amanuensis::core::Value json_value = amanuensis::ToValue(original);
      ASSERT_TRUE(json_value.IsObject());
      ASSERT_EQUAL(json_value.Get("alpha").AsInteger(), 1LL);
      ASSERT_EQUAL(json_value.Get("beta").AsInteger(), 2LL);
    });

    IT("deserialises a string-keyed map", {
      auto original = MakeStringIntMap();
      amanuensis::core::Value json_value = amanuensis::ToValue(original);
      auto round_tripped = amanuensis::FromValue<StringIntMapType>(json_value);
      ASSERT_EQUAL(round_tripped["alpha"], 1);
      ASSERT_EQUAL(round_tripped["beta"], 2);
    });
  });
});
