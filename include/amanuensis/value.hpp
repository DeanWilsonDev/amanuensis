#pragma once

#include <cstddef>
#include <memory>
#include <stdexcept>
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

  class ObjectIterator {
  public:
    using UnderlyingIterator =
        std::vector<std::pair<std::string, Value>>::const_iterator;

    ObjectIterator() = default;
    explicit ObjectIterator(UnderlyingIterator iterator) : iterator_(iterator) {}

    const std::pair<std::string, Value>& operator*() const { return *iterator_; }
    const std::pair<std::string, Value>* operator->() const { return &(*iterator_); }

    ObjectIterator& operator++() {
      ++iterator_;
      return *this;
    }

    ObjectIterator operator++(int) {
      ObjectIterator previous = *this;
      ++iterator_;
      return previous;
    }

    bool operator==(const ObjectIterator& other) const {
      return iterator_ == other.iterator_;
    }

    bool operator!=(const ObjectIterator& other) const {
      return iterator_ != other.iterator_;
    }

  private:
    UnderlyingIterator iterator_;
  };

  ObjectIterator BeginObject() const;
  ObjectIterator EndObject() const;

  const std::vector<Value>& AsArray() const;

private:
  struct Impl;
  std::shared_ptr<Impl> impl_;
};

// -----------------------------------------------------------------------
// Error types
// -----------------------------------------------------------------------

class TypeMismatchError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

class KeyNotFoundError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

class IndexOutOfRangeError : public std::out_of_range {
public:
  using std::out_of_range::out_of_range;
};

} // namespace Amanuensis
