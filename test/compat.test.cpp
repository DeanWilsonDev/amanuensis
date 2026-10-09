#include <cimmerian/test.hpp>
#include <amanuensis/json-value.hpp>
#include <amanuensis/json.hpp>
#include <amanuensis/io/writer.hpp>

#include <string>
#include <type_traits>

static_assert(std::is_same_v<Amanuensis::JsonValue, amanuensis::JsonValue>);

DESCRIBE("Compatibility namespace", {
  IT("lets qualified names use the old Amanuensis spelling", {
    Amanuensis::JsonValue object_value = Amanuensis::Json::MakeObject();
    Amanuensis::Json::Insert(object_value, "name", Amanuensis::JsonValue{ std::string("Alice") });

    ASSERT_TRUE(amanuensis::Json::IsObject(object_value));
    ASSERT_EQUAL(amanuensis::Json::AsString(amanuensis::Json::Get(object_value, "name")),
                 std::string("Alice"));
  });

  IT("lets using-directives name the old Amanuensis namespace", {
    using namespace Amanuensis;
    JsonValue object_value = Json::MakeObject();
    Json::Insert(object_value, "count", JsonValue{ 3LL });

    WriterOptions options;
    options.pretty = false;
    options.trailingNewline = false;
    ASSERT_EQUAL(Writer::WriteToString(object_value, options), std::string("{\"count\":3}"));
  });
});
