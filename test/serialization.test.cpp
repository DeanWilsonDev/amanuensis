#include <cimmerian/test.hpp>
#include <amanuensis/serialization/serialization.hpp>
#include <amanuensis/json/reader.hpp>
#include <amanuensis/json/writer.hpp>
#include <amanuensis/core/parse-result.hpp>
#include <amanuensis/core/value.hpp>

#include <cmath>
#include <map>
#include <optional>
#include <string>
#include <variant>
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

namespace amanuensis {
template <> struct SerialTraits<Vec3> {
  static core::Value ToValue(const Vec3& vector)
  {
    core::Value array_value = amanuensis::core::Value::MakeArray();
    array_value.PushBack(core::Value{vector.x});
    array_value.PushBack(core::Value{vector.y});
    array_value.PushBack(core::Value{vector.z});
    return array_value;
  }
  static Vec3 FromValue(const core::Value& value)
  {
    return {value.At(0).AsDouble(), value.At(1).AsDouble(), value.At(2).AsDouble()};
  }
};
} // namespace amanuensis

struct ConfigEntry {
  std::string key;
  int value;
  std::optional<std::string> description;
};

AMANUENSIS_SERIALISABLE(ConfigEntry, key, value, description);

struct Transform {
  float x;
  float y;
  double weight;
};

AMANUENSIS_SERIALISABLE(Transform, x, y, weight);

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
      amanuensis::core::Value json_value = amanuensis::ToValue(original);

      ASSERT_TRUE(json_value.IsObject());
      ASSERT_EQUAL(json_value.Get("qualifiedName").AsString(), std::string("math::Add"));
      ASSERT_EQUAL(json_value.Get("startLine").AsInteger(), 10LL);
      ASSERT_EQUAL(json_value.Get("endLine").AsInteger(), 14LL);
      ASSERT_EQUAL(json_value.Get("linesTotal").AsInteger(), 5LL);
      ASSERT_EQUAL(json_value.Get("linesCovered").AsInteger(), 5LL);
      ASSERT_EQUAL(json_value.Get("executionCount").AsInteger(), 3LL);
    });

    IT("deserialises JSON back to a struct", {
      auto original = MakeTestCoverage();
      amanuensis::core::Value json_value = amanuensis::ToValue(original);
      PerFunctionCoverage round_tripped = amanuensis::FromValue<PerFunctionCoverage>(json_value);

      ASSERT_EQUAL(round_tripped.qualifiedName, std::string("math::Add"));
      ASSERT_EQUAL(round_tripped.startLine, 10);
      ASSERT_EQUAL(round_tripped.endLine, 14);
      ASSERT_EQUAL(round_tripped.linesTotal, 5);
      ASSERT_EQUAL(round_tripped.linesCovered, 5);
      ASSERT_EQUAL(round_tripped.executionCount, 3);
    });

    IT("uses C++ field names as JSON keys", {
      auto original = MakeMinimalCoverage();
      amanuensis::core::Value json_value = amanuensis::ToValue(original);
      ASSERT_TRUE(json_value.Contains("qualifiedName"));
      ASSERT_TRUE(json_value.Contains("startLine"));
      ASSERT_TRUE(json_value.Contains("endLine"));
    });
  });

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

  DESCRIBE("Mechanism 3: SerialTraits specialisation", {
    IT("serialises to an array", {
      auto original = MakeSimpleVec3();
      amanuensis::core::Value json_value = amanuensis::ToValue(original);
      ASSERT_TRUE(json_value.IsArray());
      ASSERT_EQUAL(json_value.Size(), 3u);
    });

    IT("round-trips through ToValue and FromValue", {
      auto original = MakeVec3();
      amanuensis::core::Value json_value = amanuensis::ToValue(original);
      Vec3 round_tripped = amanuensis::FromValue<Vec3>(json_value);

      ASSERT_TRUE(std::abs(round_tripped.x - 1.5) < 1e-15);
      ASSERT_TRUE(std::abs(round_tripped.y - (-2.5)) < 1e-15);
      ASSERT_TRUE(std::abs(round_tripped.z - 3.0) < 1e-15);
    });
  });

  DESCRIBE("TryFromValue non-throwing variant", {
    IT("succeeds on valid input", {
      auto original = MakeMinimalCoverage();
      amanuensis::core::Value json_value = amanuensis::ToValue(original);
      auto try_result = amanuensis::TryFromValue<PerFunctionCoverage>(json_value);
      ASSERT_TRUE(try_result.succeeded);
      ASSERT_EQUAL(try_result.value.qualifiedName, std::string("f"));
    });

    IT("fails on missing required fields without throwing", {
      amanuensis::core::Value empty_object = amanuensis::core::Value::MakeObject();
      auto try_result = amanuensis::TryFromValue<PerFunctionCoverage>(empty_object);
      ASSERT_FALSE(try_result.succeeded);
      ASSERT_FALSE(try_result.errorMessage.empty());
    });
  });

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

  DESCRIBE("Built-in SerialTraits: std::vector", {
    IT("serialises a vector of ints", {
      auto original = MakeIntVector();
      amanuensis::core::Value json_value = amanuensis::ToValue(original);
      ASSERT_TRUE(json_value.IsArray());
      ASSERT_EQUAL(json_value.Size(), 3u);
      ASSERT_EQUAL(json_value.At(0).AsInteger(), 10LL);
      ASSERT_EQUAL(json_value.At(2).AsInteger(), 30LL);
    });

    IT("deserialises a vector of ints", {
      auto original = MakeIntVector();
      amanuensis::core::Value json_value = amanuensis::ToValue(original);
      auto round_tripped = amanuensis::FromValue<std::vector<int>>(json_value);
      ASSERT_EQUAL(round_tripped.size(), 3u);
      ASSERT_EQUAL(round_tripped[0], 10);
      ASSERT_EQUAL(round_tripped[2], 30);
    });

    IT("handles an empty vector", {
      std::vector<int> empty_vector;
      amanuensis::core::Value json_value = amanuensis::ToValue(empty_vector);
      ASSERT_TRUE(json_value.IsArray());
      ASSERT_EQUAL(json_value.Size(), 0u);
    });

    IT("serialises a vector of user types", {
      auto vectors = MakeVec3Vector();
      amanuensis::core::Value json_value = amanuensis::ToValue(vectors);
      ASSERT_EQUAL(json_value.Size(), 2u);
      ASSERT_EQUAL(json_value.At(0).Size(), 3u);
    });
  });

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

  DESCRIBE("Built-in SerialTraits: std::map", {
    IT("serialises a string-keyed map", {
      auto original = MakeStringIntMap();
      amanuensis::core::Value json_value = amanuensis::ToValue(original);
      ASSERT_TRUE(json_value.IsObject());
      ASSERT_EQUAL(json_value.Get("alpha").AsInteger(), 1LL);
      ASSERT_EQUAL(json_value.Get("beta").AsInteger(), 2LL);
    });

    IT("deserialises a string-keyed map", {
      auto original = MakeStringIntMap();
      amanuensis::core::Value json_value = amanuensis::ToValue(original);
      auto round_tripped = amanuensis::FromValue<StringIntMapType>(json_value);
      ASSERT_EQUAL(round_tripped["alpha"], 1);
      ASSERT_EQUAL(round_tripped["beta"], 2);
    });
  });

  DESCRIBE("Optional field handling in structs", {
    IT("populates optional field when present in JSON", {
      auto parsed = amanuensis::json::Reader::ParseString(
          R"({"key":"host","value":8080,"description":"Server port"})"
      );
      REQUIRE_TRUE(parsed.succeeded);
      auto entry = amanuensis::FromValue<ConfigEntry>(parsed.value);
      ASSERT_EQUAL(entry.key, std::string("host"));
      ASSERT_EQUAL(entry.value, 8080);
      ASSERT_TRUE(entry.description.has_value());
      ASSERT_EQUAL(*entry.description, std::string("Server port"));
    });

    IT("leaves optional field empty when absent in JSON", {
      auto parsed = amanuensis::json::Reader::ParseString(R"({"key":"host","value":8080})");
      REQUIRE_TRUE(parsed.succeeded);
      auto entry = amanuensis::FromValue<ConfigEntry>(parsed.value);
      ASSERT_EQUAL(entry.key, std::string("host"));
      ASSERT_FALSE(entry.description.has_value());
    });

    IT("leaves optional field empty when null in JSON", {
      auto parsed = amanuensis::json::Reader::ParseString(
          R"({"key":"host","value":8080,"description":null})"
      );
      REQUIRE_TRUE(parsed.succeeded);
      auto entry = amanuensis::FromValue<ConfigEntry>(parsed.value);
      ASSERT_FALSE(entry.description.has_value());
    });
  });

  DESCRIBE("Extra fields in JSON", {
    IT("silently ignores extra fields during deserialisation", {
      auto parsed = amanuensis::json::Reader::ParseString(
          R"({"display_name":"W","item_count":5,"unknown_field":"ignored"})"
      );
      REQUIRE_TRUE(parsed.succeeded);
      auto result = amanuensis::TryFromValue<RenamedFields>(parsed.value);
      ASSERT_TRUE(result.succeeded);
      ASSERT_EQUAL(result.value.name, std::string("W"));
      ASSERT_EQUAL(result.value.count, 5);
    });
  });

  DESCRIBE("Full serialisation round-trip through file", {
    IT("writes and reads back a struct through JSON file", {
      auto original = MakeTestCoverage2();
      amanuensis::core::Value json_value = amanuensis::ToValue(original);

      bool write_succeeded =
          amanuensis::json::Writer::WriteToFile(json_value, "/tmp/amanuensis_serial_test.json");
      REQUIRE_TRUE(write_succeeded);

      auto parse_result = amanuensis::json::Reader::ParseFile("/tmp/amanuensis_serial_test.json");
      REQUIRE_TRUE(parse_result.succeeded);

      PerFunctionCoverage round_tripped =
          amanuensis::FromValue<PerFunctionCoverage>(parse_result.value);
      ASSERT_EQUAL(round_tripped.qualifiedName, std::string("math::Multiply"));
      ASSERT_EQUAL(round_tripped.startLine, 20);
      ASSERT_EQUAL(round_tripped.linesCovered, 8);
      ASSERT_EQUAL(round_tripped.executionCount, 7);
    });
  });
});
