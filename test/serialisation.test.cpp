#include <cimmerian/test.hpp>
#include <amanuensis/serialisation/serialization.hpp>
#include <amanuensis/io/reader.hpp>
#include <amanuensis/io/writer.hpp>
#include <amanuensis/io/parser-result.hpp>
#include <amanuensis/json.hpp>

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
    Value array_value = Json::MakeArray();
    Json::PushBack(array_value, Value{ vector.x });
    Json::PushBack(array_value, Value{ vector.y });
    Json::PushBack(array_value, Value{ vector.z });
    return array_value;
  }
  static Vec3 FromJson(const Value& value)
  {
    return {
      Json::AsDouble(Json::At(value, 0)),
      Json::AsDouble(Json::At(value, 1)),
      Json::AsDouble(Json::At(value, 2))
    };
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
// Helper functions
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
      Amanuensis::Value json_value = Amanuensis::ToJson(original);

      ASSERT_TRUE(Amanuensis::Json::IsObject(json_value));
      ASSERT_EQUAL(Amanuensis::Json::AsString(Amanuensis::Json::Get(json_value, "qualifiedName")), std::string("math::Add"));
      ASSERT_EQUAL(Amanuensis::Json::AsInteger(Amanuensis::Json::Get(json_value, "startLine")), 10LL);
      ASSERT_EQUAL(Amanuensis::Json::AsInteger(Amanuensis::Json::Get(json_value, "endLine")), 14LL);
      ASSERT_EQUAL(Amanuensis::Json::AsInteger(Amanuensis::Json::Get(json_value, "linesTotal")), 5LL);
      ASSERT_EQUAL(Amanuensis::Json::AsInteger(Amanuensis::Json::Get(json_value, "linesCovered")), 5LL);
      ASSERT_EQUAL(Amanuensis::Json::AsInteger(Amanuensis::Json::Get(json_value, "executionCount")), 3LL);
    });

    IT("deserialises JSON back to a struct", {
      auto original = MakeTestCoverage();
      Amanuensis::Value json_value = Amanuensis::ToJson(original);
      PerFunctionCoverage round_tripped = Amanuensis::FromJson<PerFunctionCoverage>(json_value);

      ASSERT_EQUAL(round_tripped.qualifiedName, std::string("math::Add"));
      ASSERT_EQUAL(round_tripped.startLine, 10);
      ASSERT_EQUAL(round_tripped.endLine, 14);
      ASSERT_EQUAL(round_tripped.linesTotal, 5);
      ASSERT_EQUAL(round_tripped.linesCovered, 5);
      ASSERT_EQUAL(round_tripped.executionCount, 3);
    });

    IT("uses C++ field names as JSON keys", {
      auto original = MakeMinimalCoverage();
      Amanuensis::Value json_value = Amanuensis::ToJson(original);
      ASSERT_TRUE(Amanuensis::Json::Contains(json_value, "qualifiedName"));
      ASSERT_TRUE(Amanuensis::Json::Contains(json_value, "startLine"));
      ASSERT_TRUE(Amanuensis::Json::Contains(json_value, "endLine"));
    });
  });

  DESCRIBE("Mechanism 2: intrusive Serialise member", {
    IT("uses custom JSON key names", {
      auto original = MakeRenamedFields();
      Amanuensis::Value json_value = Amanuensis::ToJson(original);

      ASSERT_TRUE(Amanuensis::Json::Contains(json_value, "display_name"));
      ASSERT_TRUE(Amanuensis::Json::Contains(json_value, "item_count"));
      ASSERT_FALSE(Amanuensis::Json::Contains(json_value, "name"));
      ASSERT_FALSE(Amanuensis::Json::Contains(json_value, "count"));
    });

    IT("serialises with renamed keys", {
      auto original = MakeRenamedFields();
      Amanuensis::Value json_value = Amanuensis::ToJson(original);
      ASSERT_EQUAL(Amanuensis::Json::AsString(Amanuensis::Json::Get(json_value, "display_name")), std::string("Widget"));
      ASSERT_EQUAL(Amanuensis::Json::AsInteger(Amanuensis::Json::Get(json_value, "item_count")), 42LL);
    });

    IT("deserialises with renamed keys", {
      auto original = MakeRenamedFields();
      Amanuensis::Value json_value = Amanuensis::ToJson(original);
      RenamedFields round_tripped = Amanuensis::FromJson<RenamedFields>(json_value);

      ASSERT_EQUAL(round_tripped.name, std::string("Widget"));
      ASSERT_EQUAL(round_tripped.count, 42);
    });
  });

  DESCRIBE("Mechanism 3: JsonTraits specialisation", {
    IT("serialises to an array", {
      auto original = MakeSimpleVec3();
      Amanuensis::Value json_value = Amanuensis::ToJson(original);
      ASSERT_TRUE(Amanuensis::Json::IsArray(json_value));
      ASSERT_EQUAL(Amanuensis::Json::Size(json_value), 3u);
    });

    IT("round-trips through ToJson and FromJson", {
      auto original = MakeVec3();
      Amanuensis::Value json_value = Amanuensis::ToJson(original);
      Vec3 round_tripped = Amanuensis::FromJson<Vec3>(json_value);

      ASSERT_TRUE(std::abs(round_tripped.x - 1.5) < 1e-15);
      ASSERT_TRUE(std::abs(round_tripped.y - (-2.5)) < 1e-15);
      ASSERT_TRUE(std::abs(round_tripped.z - 3.0) < 1e-15);
    });
  });

  DESCRIBE("TryFromJson non-throwing variant", {
    IT("succeeds on valid input", {
      auto original = MakeMinimalCoverage();
      Amanuensis::Value json_value = Amanuensis::ToJson(original);
      auto try_result = Amanuensis::TryFromJson<PerFunctionCoverage>(json_value);
      ASSERT_TRUE(try_result.succeeded);
      ASSERT_EQUAL(try_result.value.qualifiedName, std::string("f"));
    });

    IT("fails on missing required fields without throwing", {
      Amanuensis::Value empty_object = Amanuensis::Json::MakeObject();
      auto try_result = Amanuensis::TryFromJson<PerFunctionCoverage>(empty_object);
      ASSERT_FALSE(try_result.succeeded);
      ASSERT_FALSE(try_result.errorMessage.empty());
    });
  });

  DESCRIBE("Built-in JsonTraits: std::vector", {
    IT("serialises a vector of ints", {
      auto original = MakeIntVector();
      Amanuensis::Value json_value = Amanuensis::ToJson(original);
      ASSERT_TRUE(Amanuensis::Json::IsArray(json_value));
      ASSERT_EQUAL(Amanuensis::Json::Size(json_value), 3u);
      ASSERT_EQUAL(Amanuensis::Json::AsInteger(Amanuensis::Json::At(json_value, 0)), 10LL);
      ASSERT_EQUAL(Amanuensis::Json::AsInteger(Amanuensis::Json::At(json_value, 2)), 30LL);
    });

    IT("deserialises a vector of ints", {
      auto original = MakeIntVector();
      Amanuensis::Value json_value = Amanuensis::ToJson(original);
      auto round_tripped = Amanuensis::FromJson<std::vector<int>>(json_value);
      ASSERT_EQUAL(round_tripped.size(), 3u);
      ASSERT_EQUAL(round_tripped[0], 10);
      ASSERT_EQUAL(round_tripped[2], 30);
    });

    IT("handles an empty vector", {
      std::vector<int> empty_vector;
      Amanuensis::Value json_value = Amanuensis::ToJson(empty_vector);
      ASSERT_TRUE(Amanuensis::Json::IsArray(json_value));
      ASSERT_EQUAL(Amanuensis::Json::Size(json_value), 0u);
    });

    IT("serialises a vector of user types", {
      auto vectors = MakeVec3Vector();
      Amanuensis::Value json_value = Amanuensis::ToJson(vectors);
      ASSERT_EQUAL(Amanuensis::Json::Size(json_value), 2u);
      ASSERT_EQUAL(Amanuensis::Json::Size(Amanuensis::Json::At(json_value, 0)), 3u);
    });
  });

  DESCRIBE("Built-in JsonTraits: std::optional", {
    IT("serialises a present optional", {
      std::optional<int> present = 42;
      Amanuensis::Value json_value = Amanuensis::ToJson(present);
      ASSERT_TRUE(Amanuensis::Json::IsInteger(json_value));
      ASSERT_EQUAL(Amanuensis::Json::AsInteger(json_value), 42LL);
    });

    IT("serialises an absent optional as null", {
      std::optional<int> absent = std::nullopt;
      Amanuensis::Value json_value = Amanuensis::ToJson(absent);
      ASSERT_TRUE(Amanuensis::Json::IsNull(json_value));
    });

    IT("deserialises a present optional", {
      Amanuensis::Value json_value{ 42LL };
      auto result = Amanuensis::FromJson<std::optional<int>>(json_value);
      ASSERT_TRUE(result.has_value());
      ASSERT_EQUAL(*result, 42);
    });

    IT("deserialises null as empty optional", {
      Amanuensis::Value json_value{ nullptr };
      auto result = Amanuensis::FromJson<std::optional<int>>(json_value);
      ASSERT_FALSE(result.has_value());
    });
  });

  DESCRIBE("Built-in JsonTraits: std::map", {
    IT("serialises a string-keyed map", {
      auto original = MakeStringIntMap();
      Amanuensis::Value json_value = Amanuensis::ToJson(original);
      ASSERT_TRUE(Amanuensis::Json::IsObject(json_value));
      ASSERT_EQUAL(Amanuensis::Json::AsInteger(Amanuensis::Json::Get(json_value, "alpha")), 1LL);
      ASSERT_EQUAL(Amanuensis::Json::AsInteger(Amanuensis::Json::Get(json_value, "beta")), 2LL);
    });

    IT("deserialises a string-keyed map", {
      auto original = MakeStringIntMap();
      Amanuensis::Value json_value = Amanuensis::ToJson(original);
      auto round_tripped = Amanuensis::FromJson<StringIntMapType>(json_value);
      ASSERT_EQUAL(round_tripped["alpha"], 1);
      ASSERT_EQUAL(round_tripped["beta"], 2);
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
      auto parsed = Amanuensis::Reader::ParseString(
          R"({"key":"host","value":8080,"description":null})"
      );
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
      Amanuensis::Value json_value = Amanuensis::ToJson(original);

      bool write_succeeded =
          Amanuensis::Writer::WriteToFile(json_value, "/tmp/amanuensis_serial_test.json");
      REQUIRE_TRUE(write_succeeded);

      auto parse_result = Amanuensis::Reader::ParseFile("/tmp/amanuensis_serial_test.json");
      REQUIRE_TRUE(parse_result.succeeded);

      PerFunctionCoverage round_tripped =
          Amanuensis::FromJson<PerFunctionCoverage>(parse_result.value);
      ASSERT_EQUAL(round_tripped.qualifiedName, std::string("math::Multiply"));
      ASSERT_EQUAL(round_tripped.startLine, 20);
      ASSERT_EQUAL(round_tripped.linesCovered, 8);
      ASSERT_EQUAL(round_tripped.executionCount, 7);
    });
  });
});
