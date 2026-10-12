#include <cimmerian/test.hpp>
#include <amanuensis/serialization/serialization.hpp>

#include <optional>
#include <variant>

DESCRIBE("Serialisation", {
  DESCRIBE("Built-in SerialTraits: std::optional", {
    IT("serialises a present optional", {
      std::optional<int> present = 42;
      amanuensis::core::Value json_value = amanuensis::ToValue(present);
      ASSERT_TRUE(json_value.IsInteger());
      ASSERT_EQUAL(json_value.AsInteger(), 42LL);
    });

    IT("serialises an absent optional as null", {
      std::optional<int> absent = std::nullopt;
      amanuensis::core::Value json_value = amanuensis::ToValue(absent);
      ASSERT_TRUE(json_value.IsNull());
    });

    IT("deserialises a present optional", {
      amanuensis::core::Value json_value{42LL};
      auto result = amanuensis::FromValue<std::optional<int>>(json_value);
      ASSERT_TRUE(result.has_value());
      ASSERT_EQUAL(*result, 42);
    });

    IT("deserialises null as empty optional", {
      amanuensis::core::Value json_value{std::monostate()};
      auto result = amanuensis::FromValue<std::optional<int>>(json_value);
      ASSERT_FALSE(result.has_value());
    });
  });
});
