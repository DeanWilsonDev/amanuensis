#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <variant>
#include "amanuensis/core/ordered-map.hpp"

namespace amanuensis::core {

enum class ValueType { Null, Boolean, Integer, Double, String, Array, Object };

class ObjectIterator;

// A JSON-shaped value: null, a boolean, an Integer (long long), a Double, a
// string, an array, or an object that keeps its keys in insertion order.
//
// The accessors check the type and throw TypeMismatchError on the wrong one.
// At throws IndexOutOfRangeError and Get throws KeyNotFoundError when there's
// nothing there; Find returns nullptr instead.
struct Value {
  using DataType = std::variant<
      std::monostate,
      bool,
      long long,
      double,
      std::string,
      std::vector<Value>,
      OrderedMap<Value>>;
  DataType data;

  static Value MakeArray();
  static Value MakeObject();

  ValueType GetType() const;
  bool IsNull() const;
  bool IsBoolean() const;
  bool IsInteger() const;
  bool IsDouble() const;
  // An Integer or a Double.
  bool IsNumber() const;
  bool IsString() const;
  bool IsArray() const;
  bool IsObject() const;

  bool AsBoolean() const;
  long long AsInteger() const;
  double AsDouble() const;
  const std::string& AsString() const;
  const std::vector<Value>& AsArray() const;

  // Arrays
  void PushBack(Value element);
  const Value& At(std::size_t index) const;
  Value& At(std::size_t index);

  // The element count of an Array, or the entry count of an Object.
  std::size_t Size() const;

  // Objects. Insert replaces the value of a key that's already there, in
  // place.
  void Insert(std::string key, Value element);
  // Removes key, keeping the other keys in order. Returns whether it was
  // there.
  bool Erase(const std::string& key);
  bool Contains(const std::string& key) const;
  const Value& Get(const std::string& key) const;
  Value& Get(const std::string& key);
  const Value* Find(const std::string& key) const;
  ObjectIterator BeginObject() const;
  ObjectIterator EndObject() const;
};

} // namespace amanuensis::core

// BeginObject and EndObject return it, so it comes with Value.
#include "amanuensis/core/object-iterator.hpp"
