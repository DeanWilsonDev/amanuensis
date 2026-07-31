#include <cimmerian/test.hpp>
#include <amanuensis/value.hpp>
#include <amanuensis/json.hpp>
#include <amanuensis/io/reader.hpp>
#include <amanuensis/io/writer.hpp>
#include <amanuensis/io/parse-result.hpp>

#include <string>
#include <vector>

static std::vector<std::string> CollectKeys(const Amanuensis::Value& object_value)
{
  std::vector<std::string> keys;
  for (auto iterator = Amanuensis::Json::BeginObject(object_value);
       iterator != Amanuensis::Json::EndObject(object_value);
       ++iterator) {
    keys.push_back(iterator->first);
  }
  return keys;
}

DESCRIBE("Insertion order", {
  DESCRIBE("Programmatic construction", {
    IT("preserves insertion order for three keys", {
      Amanuensis::Value object_value = Amanuensis::Json::MakeObject();
      Amanuensis::Json::Insert(object_value, "zebra", Amanuensis::Value{ 1LL });
      Amanuensis::Json::Insert(object_value, "apple", Amanuensis::Value{ 2LL });
      Amanuensis::Json::Insert(object_value, "mango", Amanuensis::Value{ 3LL });

      auto keys = CollectKeys(object_value);
      ASSERT_EQUAL(keys.size(), 3u);
      ASSERT_EQUAL(keys[0], std::string("zebra"));
      ASSERT_EQUAL(keys[1], std::string("apple"));
      ASSERT_EQUAL(keys[2], std::string("mango"));
    });

    IT("overwrites value but preserves position on duplicate key", {
      Amanuensis::Value object_value = Amanuensis::Json::MakeObject();
      Amanuensis::Json::Insert(object_value, "first",  Amanuensis::Value{ 1LL });
      Amanuensis::Json::Insert(object_value, "second", Amanuensis::Value{ 2LL });
      Amanuensis::Json::Insert(object_value, "first",  Amanuensis::Value{ 99LL });

      auto keys = CollectKeys(object_value);
      ASSERT_EQUAL(keys.size(), 2u);
      ASSERT_EQUAL(keys[0], std::string("first"));
      ASSERT_EQUAL(keys[1], std::string("second"));
      ASSERT_EQUAL(Amanuensis::Json::AsInteger(Amanuensis::Json::Get(object_value, "first")), 99LL);
    });
  });

  DESCRIBE("Parsed from source", {
    IT("preserves source key order from parsed JSON", {
      auto result = Amanuensis::Reader::ParseString("{\"z\": 1, \"a\": 2, \"m\": 3}");
      REQUIRE_TRUE(result.succeeded);

      auto keys = CollectKeys(result.value);
      ASSERT_EQUAL(keys[0], std::string("z"));
      ASSERT_EQUAL(keys[1], std::string("a"));
      ASSERT_EQUAL(keys[2], std::string("m"));
    });

    IT("preserves order of a larger parsed object", {
      auto result = Amanuensis::Reader::ParseString(
          R"({"delta":4,"alpha":1,"charlie":3,"bravo":2,"echo":5})"
      );
      REQUIRE_TRUE(result.succeeded);

      auto keys = CollectKeys(result.value);
      ASSERT_EQUAL(keys[0], std::string("delta"));
      ASSERT_EQUAL(keys[1], std::string("alpha"));
      ASSERT_EQUAL(keys[2], std::string("charlie"));
      ASSERT_EQUAL(keys[3], std::string("bravo"));
      ASSERT_EQUAL(keys[4], std::string("echo"));
    });
  });

  DESCRIBE("Round-trip order stability", {
    IT("key order survives parse-write-parse cycle", {
      std::string original_json = R"({"z":1,"a":2,"m":3})";
      auto first_parse = Amanuensis::Reader::ParseString(original_json);
      REQUIRE_TRUE(first_parse.succeeded);

      Amanuensis::WriterOptions minified_options;
      minified_options.pretty = false;
      minified_options.trailingNewline = false;
      std::string rewritten = Amanuensis::Writer::WriteToString(first_parse.value, minified_options);

      auto second_parse = Amanuensis::Reader::ParseString(rewritten);
      REQUIRE_TRUE(second_parse.succeeded);

      auto first_keys  = CollectKeys(first_parse.value);
      auto second_keys = CollectKeys(second_parse.value);

      ASSERT_EQUAL(first_keys.size(), second_keys.size());
      for (std::size_t i = 0; i < first_keys.size(); ++i) {
        ASSERT_EQUAL(first_keys[i], second_keys[i]);
      }
    });

    IT("key order survives multiple round-trip cycles", {
      std::string json = R"({"c":3,"a":1,"b":2})";
      for (int cycle = 0; cycle < 5; ++cycle) {
        auto parsed = Amanuensis::Reader::ParseString(json);
        REQUIRE_TRUE(parsed.succeeded);

        Amanuensis::WriterOptions minified_options;
        minified_options.pretty = false;
        minified_options.trailingNewline = false;
        json = Amanuensis::Writer::WriteToString(parsed.value, minified_options);
      }

      auto final_parse = Amanuensis::Reader::ParseString(json);
      REQUIRE_TRUE(final_parse.succeeded);

      auto keys = CollectKeys(final_parse.value);
      ASSERT_EQUAL(keys[0], std::string("c"));
      ASSERT_EQUAL(keys[1], std::string("a"));
      ASSERT_EQUAL(keys[2], std::string("b"));
    });
  });
});
