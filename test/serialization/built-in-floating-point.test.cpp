#include <cimmerian/test.hpp>
#include "serialization-support.hpp"
#include <amanuensis/json/reader.hpp>
#include <amanuensis/json/writer.hpp>

#include <string>

using namespace serialization_test;

DESCRIBE("Serialisation", {
  DESCRIBE("Built-in SerialTraits: floating point", {
    IT("round-trips a float exactly through a Value", {
      amanuensis::core::Value json_value = amanuensis::ToValue(0.1f);
      ASSERT_TRUE(json_value.IsDouble());
      ASSERT_EQUAL(amanuensis::FromValue<float>(json_value), 0.1f);
    });

    IT("round-trips float fields through JSON text", {
      Transform original{0.1f, -2.5f, 0.3};
      std::string text = amanuensis::json::Writer::WriteToString(amanuensis::ToValue(original));
      auto parsed = amanuensis::json::Reader::ParseString(text);
      REQUIRE_TRUE(parsed.succeeded);
      Transform round_tripped = amanuensis::FromValue<Transform>(parsed.value);
      ASSERT_EQUAL(round_tripped.x, 0.1f);
      ASSERT_EQUAL(round_tripped.y, -2.5f);
      ASSERT_EQUAL(round_tripped.weight, 0.3);
    });

    IT("reads an Integer into a double or a float", {
      ASSERT_EQUAL(amanuensis::FromValue<double>(amanuensis::core::Value{1LL}), 1.0);
      ASSERT_EQUAL(amanuensis::FromValue<float>(amanuensis::core::Value{-2LL}), -2.0f);
    });

    IT("reads hand-typed integers into floating-point fields", {
      auto parsed = amanuensis::json::Reader::ParseString(R"({"x":1,"y":-2,"weight":3})");
      REQUIRE_TRUE(parsed.succeeded);
      Transform transform = amanuensis::FromValue<Transform>(parsed.value);
      ASSERT_EQUAL(transform.x, 1.0f);
      ASSERT_EQUAL(transform.y, -2.0f);
      ASSERT_EQUAL(transform.weight, 3.0);
    });

    IT("still rejects a string", {
      ASSERT_THROWS(
          amanuensis::FromValue<double>(amanuensis::core::Value{std::string("1")}),
          amanuensis::core::TypeMismatchError
      );
    });
  });
});
