#include <cimmerian/test.hpp>
#include <amanuensis/json-value.hpp>
#include <amanuensis/io/json-parse-result.hpp>
#include <amanuensis/io/json-parse-error.hpp>
#include <amanuensis/serialization/json-traits.hpp>
#include <amanuensis/serialization/json-traits-std.hpp>
#include <amanuensis/json.hpp>
#include <amanuensis/io/writer.hpp>

#include <string>
#include <type_traits>
#include <vector>

static_assert(std::is_same_v<Amanuensis::JsonValue, amanuensis::JsonValue>);
static_assert(std::is_same_v<amanuensis::JsonValue, amanuensis::Value>);
static_assert(std::is_same_v<amanuensis::JsonValueType, amanuensis::ValueType>);
static_assert(std::is_same_v<amanuensis::JsonParseResult, amanuensis::ParseResult>);
static_assert(std::is_same_v<amanuensis::JsonParseError, amanuensis::ParseError>);
static_assert(std::is_same_v<amanuensis::FromJsonResult<int>, amanuensis::FromValueResult<int>>);

// Specialised the old way: JsonTraits, with ToJson and FromJson members.
struct LegacyPoint {
  long long x, y;
};

namespace amanuensis {
template <> struct JsonTraits<LegacyPoint> {
  static JsonValue ToJson(const LegacyPoint& point)
  {
    JsonValue array_value = Json::MakeArray();
    Json::PushBack(array_value, JsonValue{point.x});
    Json::PushBack(array_value, JsonValue{point.y});
    return array_value;
  }
  static LegacyPoint FromJson(const JsonValue& value)
  {
    return {Json::AsInteger(Json::At(value, 0)), Json::AsInteger(Json::At(value, 1))};
  }
};
} // namespace amanuensis

static_assert(amanuensis::HasJsonTraits<LegacyPoint>::value);
static_assert(amanuensis::HasSerialTraits<LegacyPoint>::value);

static LegacyPoint MakeLegacyPoint(long long x, long long y)
{
  return {x, y};
}

static std::vector<LegacyPoint> MakeLegacyPoints()
{
  return {{1, 2}, {5, 6}};
}

DESCRIBE("Compatibility names", {
  IT("lets qualified names use the old Amanuensis spelling", {
    Amanuensis::JsonValue object_value = Amanuensis::Json::MakeObject();
    Amanuensis::Json::Insert(object_value, "name", Amanuensis::JsonValue{std::string("Alice")});

    ASSERT_TRUE(amanuensis::Json::IsObject(object_value));
    ASSERT_EQUAL(
        amanuensis::Json::AsString(amanuensis::Json::Get(object_value, "name")),
        std::string("Alice")
    );
  });

  IT("lets using-directives name the old Amanuensis namespace", {
    using namespace Amanuensis;
    JsonValue object_value = Json::MakeObject();
    Json::Insert(object_value, "count", JsonValue{3LL});

    WriterOptions options;
    options.pretty = false;
    options.trailingNewline = false;
    ASSERT_EQUAL(Writer::WriteToString(object_value, options), std::string("{\"count\":3}"));
  });

  IT("picks up an old JsonTraits specialisation through SerialTraits", {
    LegacyPoint original = MakeLegacyPoint(3, 4);
    amanuensis::Value value = amanuensis::ToValue(original);
    ASSERT_EQUAL(amanuensis::Json::AsInteger(amanuensis::Json::At(value, 1)), 4LL);

    LegacyPoint round_tripped = amanuensis::FromValue<LegacyPoint>(value);
    ASSERT_EQUAL(round_tripped.x, 3LL);
    ASSERT_EQUAL(round_tripped.y, 4LL);
  });

  IT("forwards ToJson, FromJson and TryFromJson to the new names", {
    std::vector<LegacyPoint> original = MakeLegacyPoints();
    amanuensis::Value value = amanuensis::ToJson(original);
    auto round_tripped = amanuensis::FromJson<std::vector<LegacyPoint>>(value);
    ASSERT_EQUAL(round_tripped.size(), 2u);
    ASSERT_EQUAL(round_tripped[1].x, 5LL);

    amanuensis::FromJsonResult<LegacyPoint> failed =
        amanuensis::TryFromJson<LegacyPoint>(amanuensis::Json::MakeObject());
    ASSERT_FALSE(failed.succeeded);
  });
});
