#include <cimmerian/test.hpp>
#include <amanuensis/serialisation/serialization.hpp>
#include <amanuensis/io/reader.hpp>
#include <amanuensis/io/writer.hpp>
#include <amanuensis/io/parser-result.hpp>

#include <cmath>
#include <map>
#include <optional>
#include <string>
#include <vector>

// -----------------------------------------------------------------------
// Test types — defined at file scope (outside macros)
// -----------------------------------------------------------------------

struct PerFunctionCoverage {
  std::string qualifiedName;
  int startLine;
  int endLine;
  int linesTotal;
  int linesCovered;
  int executionCount;
};

AMANUENSIS_SERIALISABLE(
    PerFunctionCoverage,
    qualifiedName,
    startLine,
    endLine,
    linesTotal,
    linesCovered,
    executionCount
);

struct RenamedFields {
  std::string name;
  int count;

  template <typename Archive> void Serialise(Archive& archive)
  {
    archive.Field("display_name", name);
    archive.Field("item_count", count);
  }
};

struct Vec3 {
  double x, y, z;
};

namespace Amanuensis {
template <> struct JsonTraits<Vec3> {
  static Value ToJson(const Vec3& vector)
  {
    auto arrayValue = Value::MakeArray();
    arrayValue.PushBack(vector.x);
    arrayValue.PushBack(vector.y);
    arrayValue.PushBack(vector.z);
    return arrayValue;
  }
  static Vec3 FromJson(const Value& value)
  {
    return {value.At(0).AsDouble(), value.At(1).AsDouble(), value.At(2).AsDouble()};
  }
};
} // namespace Amanuensis

struct ConfigEntry {
  std::string key;
  int value;
  std::optional<std::string> description;
};

AMANUENSIS_SERIALISABLE(ConfigEntry, key, value, description);

// -----------------------------------------------------------------------
// Helper functions that construct test data outside of macro bodies
// to avoid bare commas confusing the preprocessor.
// -----------------------------------------------------------------------

static PerFunctionCoverage MakeTestCoverage()
{
  PerFunctionCoverage pfc;
  pfc.qualifiedName = "math::Add";
  pfc.startLine = 10;
  pfc.endLine = 14;
  pfc.linesTotal = 5;
  pfc.linesCovered = 5;
  pfc.executionCount = 3;
  return pfc;
}

static PerFunctionCoverage MakeTestCoverage2()
{
  PerFunctionCoverage pfc;
  pfc.qualifiedName = "math::Multiply";
  pfc.startLine = 20;
  pfc.endLine = 30;
  pfc.linesTotal = 11;
  pfc.linesCovered = 8;
  pfc.executionCount = 7;
  return pfc;
}

static PerFunctionCoverage MakeMinimalCoverage()
{
  PerFunctionCoverage pfc;
  pfc.qualifiedName = "f";
  pfc.startLine = 1;
  pfc.endLine = 2;
  pfc.linesTotal = 3;
  pfc.linesCovered = 4;
  pfc.executionCount = 5;
  return pfc;
}

static RenamedFields MakeRenamedFields()
{
  RenamedFields rf;
  rf.name = "Widget";
  rf.count = 42;
  return rf;
}

static Vec3 MakeVec3()
{
  Vec3 v;
  v.x = 1.5;
  v.y = -2.5;
  v.z = 3.0;
  return v;
}

static Vec3 MakeSimpleVec3()
{
  Vec3 v;
  v.x = 1.0;
  v.y = 2.0;
  v.z = 3.0;
  return v;
}

static std::vector<int> MakeIntVector()
{
  std::vector<int> v;
  v.push_back(10);
  v.push_back(20);
  v.push_back(30);
  return v;
}

static std::vector<Vec3> MakeVec3Vector()
{
  std::vector<Vec3> v;
  Vec3 a;
  a.x = 1;
  a.y = 2;
  a.z = 3;
  Vec3 b;
  b.x = 4;
  b.y = 5;
  b.z = 6;
  v.push_back(a);
  v.push_back(b);
  return v;
}

static std::map<std::string, int> MakeStringIntMap()
{
  std::map<std::string, int> m;
  m["alpha"] = 1;
  m["beta"] = 2;
  return m;
}

using StringIntMapType = std::map<std::string, int>;

// -----------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------

DESCRIBE("Serialisation", {
  DESCRIBE("Mechanism 1: AMANUENSIS_SERIALISABLE macro", {
    IT("serialises a struct to JSON", {
      auto original = MakeTestCoverage();
      Amanuensis::Value jsonValue = Amanuensis::ToJson(original);

      ASSERT_TRUE(jsonValue.IsObject());
      ASSERT_EQUAL(jsonValue.Get("qualifiedName").AsString(), std::string("math::Add"));
      ASSERT_EQUAL(jsonValue.Get("startLine").AsInteger(), 10LL);
      ASSERT_EQUAL(jsonValue.Get("endLine").AsInteger(), 14LL);
      ASSERT_EQUAL(jsonValue.Get("linesTotal").AsInteger(), 5LL);
      ASSERT_EQUAL(jsonValue.Get("linesCovered").AsInteger(), 5LL);
      ASSERT_EQUAL(jsonValue.Get("executionCount").AsInteger(), 3LL);
    });

    IT("deserialises JSON back to a struct", {
      auto original = MakeTestCoverage();
      Amanuensis::Value jsonValue = Amanuensis::ToJson(original);
      PerFunctionCoverage roundTripped = Amanuensis::FromJson<PerFunctionCoverage>(jsonValue);

      ASSERT_EQUAL(roundTripped.qualifiedName, std::string("math::Add"));
      ASSERT_EQUAL(roundTripped.startLine, 10);
      ASSERT_EQUAL(roundTripped.endLine, 14);
      ASSERT_EQUAL(roundTripped.linesTotal, 5);
      ASSERT_EQUAL(roundTripped.linesCovered, 5);
      ASSERT_EQUAL(roundTripped.executionCount, 3);
    });

    IT("uses C++ field names as JSON keys", {
      auto original = MakeMinimalCoverage();
      Amanuensis::Value jsonValue = Amanuensis::ToJson(original);
      ASSERT_TRUE(jsonValue.Contains("qualifiedName"));
      ASSERT_TRUE(jsonValue.Contains("startLine"));
      ASSERT_TRUE(jsonValue.Contains("endLine"));
    });
  });

  DESCRIBE("Mechanism 2: intrusive Serialise member", {
    IT("uses custom JSON key names", {
      auto original = MakeRenamedFields();
      Amanuensis::Value jsonValue = Amanuensis::ToJson(original);

      ASSERT_TRUE(jsonValue.Contains("display_name"));
      ASSERT_TRUE(jsonValue.Contains("item_count"));
      ASSERT_FALSE(jsonValue.Contains("name"));
      ASSERT_FALSE(jsonValue.Contains("count"));
    });

    IT("serialises with renamed keys", {
      auto original = MakeRenamedFields();
      Amanuensis::Value jsonValue = Amanuensis::ToJson(original);
      ASSERT_EQUAL(jsonValue.Get("display_name").AsString(), std::string("Widget"));
      ASSERT_EQUAL(jsonValue.Get("item_count").AsInteger(), 42LL);
    });

    IT("deserialises with renamed keys", {
      auto original = MakeRenamedFields();
      Amanuensis::Value jsonValue = Amanuensis::ToJson(original);
      RenamedFields roundTripped = Amanuensis::FromJson<RenamedFields>(jsonValue);

      ASSERT_EQUAL(roundTripped.name, std::string("Widget"));
      ASSERT_EQUAL(roundTripped.count, 42);
    });
  });

  DESCRIBE("Mechanism 3: JsonTraits specialisation", {
    IT("serialises to an array", {
      auto original = MakeSimpleVec3();
      Amanuensis::Value jsonValue = Amanuensis::ToJson(original);
      ASSERT_TRUE(jsonValue.IsArray());
      ASSERT_EQUAL(jsonValue.Size(), 3u);
    });

    IT("round-trips through ToJson and FromJson", {
      auto original = MakeVec3();
      Amanuensis::Value jsonValue = Amanuensis::ToJson(original);
      Vec3 roundTripped = Amanuensis::FromJson<Vec3>(jsonValue);

      ASSERT_TRUE(std::abs(roundTripped.x - 1.5) < 1e-15);
      ASSERT_TRUE(std::abs(roundTripped.y - (-2.5)) < 1e-15);
      ASSERT_TRUE(std::abs(roundTripped.z - 3.0) < 1e-15);
    });
  });

  DESCRIBE("TryFromJson non-throwing variant", {
    IT("succeeds on valid input", {
      auto original = MakeMinimalCoverage();
      Amanuensis::Value jsonValue = Amanuensis::ToJson(original);
      auto tryResult = Amanuensis::TryFromJson<PerFunctionCoverage>(jsonValue);
      ASSERT_TRUE(tryResult.succeeded);
      ASSERT_EQUAL(tryResult.value.qualifiedName, std::string("f"));
    });

    IT("fails on missing required fields without throwing", {
      Amanuensis::Value emptyObject = Amanuensis::Value::MakeObject();
      auto tryResult = Amanuensis::TryFromJson<PerFunctionCoverage>(emptyObject);
      ASSERT_FALSE(tryResult.succeeded);
      ASSERT_FALSE(tryResult.errorMessage.empty());
    });
  });

  DESCRIBE("Built-in JsonTraits: std::vector", {
    IT("serialises a vector of ints", {
      auto original = MakeIntVector();
      Amanuensis::Value jsonValue = Amanuensis::ToJson(original);
      ASSERT_TRUE(jsonValue.IsArray());
      ASSERT_EQUAL(jsonValue.Size(), 3u);
      ASSERT_EQUAL(jsonValue.At(0).AsInteger(), 10LL);
      ASSERT_EQUAL(jsonValue.At(2).AsInteger(), 30LL);
    });

    IT("deserialises a vector of ints", {
      auto original = MakeIntVector();
      Amanuensis::Value jsonValue = Amanuensis::ToJson(original);
      auto roundTripped = Amanuensis::FromJson<std::vector<int>>(jsonValue);
      ASSERT_EQUAL(roundTripped.size(), 3u);
      ASSERT_EQUAL(roundTripped[0], 10);
      ASSERT_EQUAL(roundTripped[2], 30);
    });

    IT("handles an empty vector", {
      std::vector<int> emptyVector;
      Amanuensis::Value jsonValue = Amanuensis::ToJson(emptyVector);
      ASSERT_TRUE(jsonValue.IsArray());
      ASSERT_EQUAL(jsonValue.Size(), 0u);
    });

    IT("serialises a vector of user types", {
      auto vectors = MakeVec3Vector();
      Amanuensis::Value jsonValue = Amanuensis::ToJson(vectors);
      ASSERT_EQUAL(jsonValue.Size(), 2u);
      ASSERT_EQUAL(jsonValue.At(0).Size(), 3u);
    });
  });

  DESCRIBE("Built-in JsonTraits: std::optional", {
    IT("serialises a present optional", {
      std::optional<int> present = 42;
      Amanuensis::Value jsonValue = Amanuensis::ToJson(present);
      ASSERT_TRUE(jsonValue.IsInteger());
      ASSERT_EQUAL(jsonValue.AsInteger(), 42LL);
    });

    IT("serialises an absent optional as null", {
      std::optional<int> absent = std::nullopt;
      Amanuensis::Value jsonValue = Amanuensis::ToJson(absent);
      ASSERT_TRUE(jsonValue.IsNull());
    });

    IT("deserialises a present optional", {
      Amanuensis::Value jsonValue(42);
      auto result = Amanuensis::FromJson<std::optional<int>>(jsonValue);
      ASSERT_TRUE(result.has_value());
      ASSERT_EQUAL(*result, 42);
    });

    IT("deserialises null as empty optional", {
      Amanuensis::Value jsonValue;
      auto result = Amanuensis::FromJson<std::optional<int>>(jsonValue);
      ASSERT_FALSE(result.has_value());
    });
  });

  DESCRIBE("Built-in JsonTraits: std::map", {
    IT("serialises a string-keyed map", {
      auto original = MakeStringIntMap();
      Amanuensis::Value jsonValue = Amanuensis::ToJson(original);
      ASSERT_TRUE(jsonValue.IsObject());
      ASSERT_EQUAL(jsonValue.Get("alpha").AsInteger(), 1LL);
      ASSERT_EQUAL(jsonValue.Get("beta").AsInteger(), 2LL);
    });

    IT("deserialises a string-keyed map", {
      auto original = MakeStringIntMap();
      Amanuensis::Value jsonValue = Amanuensis::ToJson(original);
      auto roundTripped = Amanuensis::FromJson<StringIntMapType>(jsonValue);
      ASSERT_EQUAL(roundTripped["alpha"], 1);
      ASSERT_EQUAL(roundTripped["beta"], 2);
    });
  });

  DESCRIBE("Optional field handling in structs", {
    IT("populates optional field when present in JSON", {
      auto parsed = Amanuensis::Reader::ParseString(
          R"({"key":"host","value":8080,"description":"Server port"})"
      );
      REQUIRE_TRUE(parsed.succeeded);
      auto entry = Amanuensis::FromJson<ConfigEntry>(parsed.value);
      ASSERT_EQUAL(entry.key, std::string("host"));
      ASSERT_EQUAL(entry.value, 8080);
      ASSERT_TRUE(entry.description.has_value());
      ASSERT_EQUAL(*entry.description, std::string("Server port"));
    });

    IT("leaves optional field empty when absent in JSON", {
      auto parsed = Amanuensis::Reader::ParseString(R"({"key":"host","value":8080})");
      REQUIRE_TRUE(parsed.succeeded);
      auto entry = Amanuensis::FromJson<ConfigEntry>(parsed.value);
      ASSERT_EQUAL(entry.key, std::string("host"));
      ASSERT_FALSE(entry.description.has_value());
    });

    IT("leaves optional field empty when null in JSON", {
      auto parsed =
          Amanuensis::Reader::ParseString(R"({"key":"host","value":8080,"description":null})");
      REQUIRE_TRUE(parsed.succeeded);
      auto entry = Amanuensis::FromJson<ConfigEntry>(parsed.value);
      ASSERT_FALSE(entry.description.has_value());
    });
  });

  DESCRIBE("Extra fields in JSON", {
    IT("silently ignores extra fields during deserialisation", {
      auto parsed = Amanuensis::Reader::ParseString(
          R"({"display_name":"W","item_count":5,"unknown_field":"ignored"})"
      );
      REQUIRE_TRUE(parsed.succeeded);
      auto result = Amanuensis::TryFromJson<RenamedFields>(parsed.value);
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(result.value.name, std::string("W"));
      ASSERT_EQUAL(result.value.count, 5);
    });
  });

  DESCRIBE("Full serialisation round-trip through file", {
    IT("writes and reads back a struct through JSON file", {
      auto original = MakeTestCoverage2();
      Amanuensis::Value jsonValue = Amanuensis::ToJson(original);

      bool writeSucceeded =
          Amanuensis::Writer::WriteToFile(jsonValue, "/tmp/amanuensis_serial_test.json");
      REQUIRE_TRUE(writeSucceeded);

      auto parseResult = Amanuensis::Reader::ParseFile("/tmp/amanuensis_serial_test.json");
      REQUIRE_TRUE(parseResult.succeeded);

      PerFunctionCoverage roundTripped =
          Amanuensis::FromJson<PerFunctionCoverage>(parseResult.value);
      ASSERT_EQUAL(roundTripped.qualifiedName, std::string("math::Multiply"));
      ASSERT_EQUAL(roundTripped.startLine, 20);
      ASSERT_EQUAL(roundTripped.linesCovered, 8);
      ASSERT_EQUAL(roundTripped.executionCount, 7);
    });
  });
});
