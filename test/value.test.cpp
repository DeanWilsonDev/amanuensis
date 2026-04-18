#include "cimmerian/test.hpp"
#include "amanuensis/value.hpp"
#include "amanuensis/reader.hpp"
#include "amanuensis/writer.hpp"

#include <string>
#include <vector>

// Helper to collect keys from an object in iteration order
static std::vector<std::string> CollectKeys(const Amanuensis::Value& objectValue)
{
  std::vector<std::string> keys;
  for (auto iterator = objectValue.BeginObject(); iterator != objectValue.EndObject(); ++iterator) {
    keys.push_back(iterator->first);
  }
  return keys;
}

DESCRIBE("Insertion order", {
  DESCRIBE("Programmatic construction", {
    IT("preserves insertion order for three keys", {
      Amanuensis::Value objectValue = Amanuensis::Value::MakeObject();
      objectValue.Insert("zebra", Amanuensis::Value(1));
      objectValue.Insert("apple", Amanuensis::Value(2));
      objectValue.Insert("mango", Amanuensis::Value(3));

      auto keys = CollectKeys(objectValue);
      ASSERT_EQUAL(keys.size(), 3u);
      ASSERT_EQUAL(keys[0], std::string("zebra"));
      ASSERT_EQUAL(keys[1], std::string("apple"));
      ASSERT_EQUAL(keys[2], std::string("mango"));
    });

    IT("overwrites value but preserves position on duplicate key", {
      Amanuensis::Value objectValue = Amanuensis::Value::MakeObject();
      objectValue.Insert("first", Amanuensis::Value(1));
      objectValue.Insert("second", Amanuensis::Value(2));
      objectValue.Insert("first", Amanuensis::Value(99));

      auto keys = CollectKeys(objectValue);
      ASSERT_EQUAL(keys.size(), 2u);
      ASSERT_EQUAL(keys[0], std::string("first"));
      ASSERT_EQUAL(keys[1], std::string("second"));
      ASSERT_EQUAL(objectValue.Get("first").AsInteger(), 99LL);
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
      std::string originalJson = R"({"z":1,"a":2,"m":3})";
      auto firstParse = Amanuensis::Reader::ParseString(originalJson);
      REQUIRE_TRUE(firstParse.succeeded);

      Amanuensis::WriterOptions minifiedOptions;
      minifiedOptions.pretty = false;
      minifiedOptions.trailingNewline = false;
      std::string rewritten = Amanuensis::Writer::WriteToString(firstParse.value, minifiedOptions);

      auto secondParse = Amanuensis::Reader::ParseString(rewritten);
      REQUIRE_TRUE(secondParse.succeeded);

      auto firstKeys = CollectKeys(firstParse.value);
      auto secondKeys = CollectKeys(secondParse.value);

      ASSERT_EQUAL(firstKeys.size(), secondKeys.size());
      for (std::size_t i = 0; i < firstKeys.size(); ++i) {
        ASSERT_EQUAL(firstKeys[i], secondKeys[i]);
      }
    });

    IT("key order survives multiple round-trip cycles", {
      std::string json = R"({"c":3,"a":1,"b":2})";
      for (int cycle = 0; cycle < 5; ++cycle) {
        auto parsed = Amanuensis::Reader::ParseString(json);
        REQUIRE_TRUE(parsed.succeeded);

        Amanuensis::WriterOptions minifiedOptions;
        minifiedOptions.pretty = false;
        minifiedOptions.trailingNewline = false;
        json = Amanuensis::Writer::WriteToString(parsed.value, minifiedOptions);
      }

      auto finalParse = Amanuensis::Reader::ParseString(json);
      REQUIRE_TRUE(finalParse.succeeded);

      auto keys = CollectKeys(finalParse.value);
      ASSERT_EQUAL(keys[0], std::string("c"));
      ASSERT_EQUAL(keys[1], std::string("a"));
      ASSERT_EQUAL(keys[2], std::string("b"));
    });
  });
});
