#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace Amanuensis {

enum class ValueType { Null, Boolean, Integer, Double, String, Array, Object };

class Value {
public:
  Value();
  Value(bool);
  Value(int);
  Value(long long);
  Value(double);
  Value(const char*);
  Value(std::string);

  static Value MakeArray();
  static Value MakeObject();

  ValueType GetType() const;
  bool IsNull() const;
  bool IsBoolean() const;
  bool IsInteger() const;
  bool IsDouble() const;
  bool IsNumber() const;
  bool IsString() const;
  bool IsArray() const;
  bool IsObject() const;

  bool AsBoolean() const;
  long long AsInteger() const;
  double AsDouble() const;
  const std::string& AsString() const;

  void PushBack(Value value);
  std::size_t Size() const;
  const Value& At(std::size_t index) const;
  Value& At(std::size_t index);

  void Insert(std::string key, Value value);
  bool Contains(const std::string& key) const;
  const Value& Get(const std::string& key) const;
  Value& Get(const std::string& key);
  const Value* Find(const std::string& key) const;

  class ObjectIterator;
  ObjectIterator BeginObject() const;
  ObjectIterator EndObject() const;

  const std::vector<Value>& AsArray() const;
};
} // namespace Amanuensis
