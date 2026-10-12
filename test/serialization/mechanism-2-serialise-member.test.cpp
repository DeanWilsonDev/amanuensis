#include <cimmerian/test.hpp>
#include "serialization-support.hpp"

#include <string>

using namespace serialization_test;

DESCRIBE("Serialisation", {
  DESCRIBE("Mechanism 2: intrusive Serialise member", {
    IT("uses custom JSON key names", {
      auto original = MakeRenamedFields();
      amanuensis::core::Value json_value = amanuensis::ToValue(original);

      ASSERT_TRUE(json_value.Contains("display_name"));
      ASSERT_TRUE(json_value.Contains("item_count"));
      ASSERT_FALSE(json_value.Contains("name"));
      ASSERT_FALSE(json_value.Contains("count"));
    });

    IT("serialises with renamed keys", {
      auto original = MakeRenamedFields();
      amanuensis::core::Value json_value = amanuensis::ToValue(original);
      ASSERT_EQUAL(json_value.Get("display_name").AsString(), std::string("Widget"));
      ASSERT_EQUAL(json_value.Get("item_count").AsInteger(), 42LL);
    });

    IT("deserialises with renamed keys", {
      auto original = MakeRenamedFields();
      amanuensis::core::Value json_value = amanuensis::ToValue(original);
      RenamedFields round_tripped = amanuensis::FromValue<RenamedFields>(json_value);

      ASSERT_EQUAL(round_tripped.name, std::string("Widget"));
      ASSERT_EQUAL(round_tripped.count, 42);
    });
  });
});
