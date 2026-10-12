#pragma once

#include <amanuensis/core/value.hpp>
#include <amanuensis/serialization/serialization.hpp>

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace serialization_test {

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

} // namespace serialization_test

namespace amanuensis {
template <> struct SerialTraits<serialization_test::Vec3> {
  static core::Value ToValue(const serialization_test::Vec3& vector)
  {
    core::Value array_value = amanuensis::core::Value::MakeArray();
    array_value.PushBack(core::Value{vector.x});
    array_value.PushBack(core::Value{vector.y});
    array_value.PushBack(core::Value{vector.z});
    return array_value;
  }
  static serialization_test::Vec3 FromValue(const core::Value& value)
  {
    return {value.At(0).AsDouble(), value.At(1).AsDouble(), value.At(2).AsDouble()};
  }
};
} // namespace amanuensis

namespace serialization_test {

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

inline PerFunctionCoverage MakeTestCoverage()
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

inline PerFunctionCoverage MakeTestCoverage2()
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

inline PerFunctionCoverage MakeMinimalCoverage()
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

inline RenamedFields MakeRenamedFields()
{
  RenamedFields rf;
  rf.name = "Widget";
  rf.count = 42;
  return rf;
}

inline Vec3 MakeVec3()
{
  Vec3 v;
  v.x = 1.5;
  v.y = -2.5;
  v.z = 3.0;
  return v;
}

inline Vec3 MakeSimpleVec3()
{
  Vec3 v;
  v.x = 1.0;
  v.y = 2.0;
  v.z = 3.0;
  return v;
}

inline std::vector<int> MakeIntVector()
{
  std::vector<int> v;
  v.push_back(10);
  v.push_back(20);
  v.push_back(30);
  return v;
}

inline std::vector<Vec3> MakeVec3Vector()
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

inline std::map<std::string, int> MakeStringIntMap()
{
  std::map<std::string, int> m;
  m["alpha"] = 1;
  m["beta"] = 2;
  return m;
}

using StringIntMapType = std::map<std::string, int>;

} // namespace serialization_test
