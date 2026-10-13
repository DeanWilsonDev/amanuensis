#include "amanuensis/core/value.hpp"
#include "amanuensis/core/errors.hpp"

#include <string>
#include <utility>
#include <variant>

namespace amanuensis::core {

static constexpr std::size_t kNullIndex = 0;
static constexpr std::size_t kBoolIndex = 1;
static constexpr std::size_t kIntegerIndex = 2;
static constexpr std::size_t kDoubleIndex = 3;
static constexpr std::size_t kStringIndex = 4;
static constexpr std::size_t kArrayIndex = 5;
static constexpr std::size_t kObjectIndex = 6;

// -----------------------------------------------------------------------
// Type inspection
// -----------------------------------------------------------------------

ValueType Value::GetType() const
{
  switch (data.index()) {
  case kNullIndex:
    return ValueType::Null;
  case kBoolIndex:
    return ValueType::Boolean;
  case kIntegerIndex:
    return ValueType::Integer;
  case kDoubleIndex:
    return ValueType::Double;
  case kStringIndex:
    return ValueType::String;
  case kArrayIndex:
    return ValueType::Array;
  case kObjectIndex:
    return ValueType::Object;
  default:
    return ValueType::Null;
  }
}

bool Value::IsNull() const
{
  return data.index() == kNullIndex;
}
bool Value::IsBoolean() const
{
  return data.index() == kBoolIndex;
}
bool Value::IsInteger() const
{
  return data.index() == kIntegerIndex;
}
bool Value::IsDouble() const
{
  return data.index() == kDoubleIndex;
}
bool Value::IsNumber() const
{
  return IsInteger() || IsDouble();
}
bool Value::IsString() const
{
  return data.index() == kStringIndex;
}
bool Value::IsArray() const
{
  return data.index() == kArrayIndex;
}
bool Value::IsObject() const
{
  return data.index() == kObjectIndex;
}

// -----------------------------------------------------------------------
// Typed accessors
// -----------------------------------------------------------------------

bool Value::AsBoolean() const
{
  if (!IsBoolean()) {
    throw TypeMismatchError("Expected Boolean, got " + std::to_string(static_cast<int>(GetType())));
  }
  return std::get<bool>(data);
}

long long Value::AsInteger() const
{
  if (!IsInteger()) {
    throw TypeMismatchError("Expected Integer, got " + std::to_string(static_cast<int>(GetType())));
  }
  return std::get<long long>(data);
}

double Value::AsDouble() const
{
  if (!IsDouble()) {
    throw TypeMismatchError("Expected Double, got " + std::to_string(static_cast<int>(GetType())));
  }
  return std::get<double>(data);
}

const std::string& Value::AsString() const
{
  if (!IsString()) {
    throw TypeMismatchError("Expected String, got " + std::to_string(static_cast<int>(GetType())));
  }
  return std::get<std::string>(data);
}

const std::vector<Value>& Value::AsArray() const
{
  if (!IsArray()) {
    throw TypeMismatchError("AsArray called on non-Array Value");
  }
  return std::get<std::vector<Value>>(data);
}

// -----------------------------------------------------------------------
// Array operations
// -----------------------------------------------------------------------

void Value::PushBack(Value element)
{
  if (!IsArray()) {
    throw TypeMismatchError("PushBack called on non-Array Value");
  }
  std::get<std::vector<Value>>(data).push_back(std::move(element));
}

std::size_t Value::Size() const
{
  if (IsArray()) {
    return std::get<std::vector<Value>>(data).size();
  }
  if (IsObject()) {
    return std::get<OrderedMap<Value>>(data).Size();
  }
  throw TypeMismatchError("Size called on non-Array, non-Object Value");
}

const Value& Value::At(std::size_t index) const
{
  if (!IsArray()) {
    throw TypeMismatchError("At(index) called on non-Array Value");
  }
  const auto& elements = std::get<std::vector<Value>>(data);
  if (index >= elements.size()) {
    throw IndexOutOfRangeError(
        "Array index " + std::to_string(index) + " out of range (size " +
        std::to_string(elements.size()) + ")"
    );
  }
  return elements[index];
}

Value& Value::At(std::size_t index)
{
  if (!IsArray()) {
    throw TypeMismatchError("At(index) called on non-Array Value");
  }
  auto& elements = std::get<std::vector<Value>>(data);
  if (index >= elements.size()) {
    throw IndexOutOfRangeError(
        "Array index " + std::to_string(index) + " out of range (size " +
        std::to_string(elements.size()) + ")"
    );
  }
  return elements[index];
}

// -----------------------------------------------------------------------
// Object operations
// -----------------------------------------------------------------------

void Value::Insert(std::string key, Value element)
{
  if (!IsObject()) {
    throw TypeMismatchError("Insert called on non-Object Value");
  }
  std::get<OrderedMap<Value>>(data).Insert(std::move(key), std::move(element));
}

bool Value::Erase(const std::string& key)
{
  if (!IsObject()) {
    throw TypeMismatchError("Erase called on non-Object Value");
  }
  return std::get<OrderedMap<Value>>(data).Erase(key);
}

bool Value::Contains(const std::string& key) const
{
  if (!IsObject()) {
    throw TypeMismatchError("Contains called on non-Object Value");
  }
  return std::get<OrderedMap<Value>>(data).Contains(key);
}

const Value& Value::Get(const std::string& key) const
{
  if (!IsObject()) {
    throw TypeMismatchError("Get called on non-Object Value");
  }
  return std::get<OrderedMap<Value>>(data).Get(key);
}

Value& Value::Get(const std::string& key)
{
  if (!IsObject()) {
    throw TypeMismatchError("Get called on non-Object Value");
  }
  return std::get<OrderedMap<Value>>(data).Get(key);
}

const Value* Value::Find(const std::string& key) const
{
  if (!IsObject()) {
    throw TypeMismatchError("Find called on non-Object Value");
  }
  return std::get<OrderedMap<Value>>(data).Find(key);
}

ObjectIterator Value::BeginObject() const
{
  if (!IsObject()) {
    throw TypeMismatchError("BeginObject called on non-Object Value");
  }
  return ObjectIterator(std::get<OrderedMap<Value>>(data).GetEntries().begin());
}

ObjectIterator Value::EndObject() const
{
  if (!IsObject()) {
    throw TypeMismatchError("EndObject called on non-Object Value");
  }
  return ObjectIterator(std::get<OrderedMap<Value>>(data).GetEntries().end());
}

Value Value::MakeArray()
{
  return Value{std::vector<Value>{}};
}

Value Value::MakeObject()
{
  return Value{OrderedMap<Value>{}};
}

} // namespace amanuensis::core
