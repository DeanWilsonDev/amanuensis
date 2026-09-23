#include <amanuensis/converter.hpp>
#include <amanuensis/io/reader.hpp>
#include <amanuensis/io/writer.hpp>
#include <cimmerian/test.hpp>

#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace {

struct AdapterValue {
  using Array = std::vector<AdapterValue>;
  using Object = std::vector<std::pair<std::string, AdapterValue>>;
  using Data = std::variant<std::monostate, bool, long long, double, std::string, Array, Object>;

  Data data;
};

} // namespace

namespace Amanuensis {

template <> struct ValueTraits<AdapterValue> {
  static JsonValueType GetType(const AdapterValue& value)
  {
    return static_cast<JsonValueType>(value.data.index());
  }

  static bool AsBoolean(const AdapterValue& value) { return std::get<bool>(value.data); }
  static long long AsInteger(const AdapterValue& value) { return std::get<long long>(value.data); }
  static double AsDouble(const AdapterValue& value) { return std::get<double>(value.data); }
  static const std::string& AsString(const AdapterValue& value)
  {
    return std::get<std::string>(value.data);
  }
  static const AdapterValue::Array& AsArray(const AdapterValue& value)
  {
    return std::get<AdapterValue::Array>(value.data);
  }
  static const AdapterValue::Object& AsObject(const AdapterValue& value)
  {
    return std::get<AdapterValue::Object>(value.data);
  }

  static AdapterValue MakeNull() { return AdapterValue{std::monostate{}}; }
  static AdapterValue MakeBoolean(bool value) { return AdapterValue{value}; }
  static AdapterValue MakeInteger(long long value) { return AdapterValue{value}; }
  static AdapterValue MakeDouble(double value) { return AdapterValue{value}; }
  static AdapterValue MakeString(const std::string& value) { return AdapterValue{value}; }
  static AdapterValue MakeArray() { return AdapterValue{AdapterValue::Array{}}; }
  static AdapterValue MakeObject() { return AdapterValue{AdapterValue::Object{}}; }

  static void PushBack(AdapterValue& target, AdapterValue element)
  {
    std::get<AdapterValue::Array>(target.data).push_back(std::move(element));
  }

  static void Insert(AdapterValue& target, const std::string& key, AdapterValue element)
  {
    std::get<AdapterValue::Object>(target.data).emplace_back(key, std::move(element));
  }
};

} // namespace Amanuensis

using ToAdapterConverter = Amanuensis::
    Converter<Amanuensis::JsonValue, AdapterValue, Amanuensis::ValueTraits<AdapterValue>>;
using ToJsonConverter = Amanuensis::Converter<AdapterValue, Amanuensis::JsonValue>;

DESCRIBE("Value conversion", {
  IT("converts recursively to and from an adapter-owned representation", {
    const std::string input =
        R"({"null":null,"bool":true,"integer":42,"double":3.5,"string":"hello","array":[1,false],"object":{"key":"value"}})";
    auto parsed = Amanuensis::Reader::ParseString(input);
    REQUIRE_TRUE(parsed.succeeded);

    auto adapted = ToAdapterConverter::ConvertValue(parsed.value);

    const auto& object = std::get<AdapterValue::Object>(adapted.data);
    ASSERT_EQUAL(object.size(), 7u);
    ASSERT_EQUAL(object[0].first, std::string("null"));
    ASSERT_EQUAL(std::get<long long>(object[2].second.data), 42LL);
    ASSERT_EQUAL(std::get<AdapterValue::Array>(object[5].second.data).size(), 2u);

    auto converted_back = ToJsonConverter::ConvertValue(adapted);
    Amanuensis::WriterOptions options;
    options.pretty = false;
    options.trailingNewline = false;
    ASSERT_EQUAL(Amanuensis::Writer::WriteToString(converted_back, options), input);
  });
});
