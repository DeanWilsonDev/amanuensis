#include "amanuensis/value.hpp"
#include "amanuensis/json.hpp"
#include "amanuensis/ordered-map.hpp"
#include "amanuensis/object-iterator.hpp"
#include "amanuensis/errors.hpp"

#include <variant>

namespace Amanuensis {

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

ValueType Json::GetType(const Value& value)
{
  switch (value.data.index()) {
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

bool Json::IsNull(const Value& value)
{
  return value.data.index() == kNullIndex;
}
bool Json::IsBoolean(const Value& value)
{
  return value.data.index() == kBoolIndex;
}
bool Json::IsInteger(const Value& value)
{
  return value.data.index() == kIntegerIndex;
}
bool Json::IsDouble(const Value& value)
{
  return value.data.index() == kDoubleIndex;
}
bool Json::IsNumber(const Value& value)
{
  return IsInteger(value) || IsDouble(value);
}
bool Json::IsString(const Value& value)
{
  return value.data.index() == kStringIndex;
}
bool Json::IsArray(const Value& value)
{
  return value.data.index() == kArrayIndex;
}
bool Json::IsObject(const Value& value)
{
  return value.data.index() == kObjectIndex;
}

// -----------------------------------------------------------------------
// Typed accessors
// -----------------------------------------------------------------------

bool Json::AsBoolean(const Value& value)
{
  if (!IsBoolean(value)) {
    throw TypeMismatchError(
        "Expected Boolean, got " + std::to_string(static_cast<int>(GetType(value)))
    );
  }
  return std::get<bool>(value.data);
}

long long Json::AsInteger(const Value& value)
{
  if (!IsInteger(value)) {
    throw TypeMismatchError(
        "Expected Integer, got " + std::to_string(static_cast<int>(GetType(value)))
    );
  }
  return std::get<long long>(value.data);
}

double Json::AsDouble(const Value& value)
{
  if (!IsDouble(value)) {
    throw TypeMismatchError(
        "Expected Double, got " + std::to_string(static_cast<int>(GetType(value)))
    );
  }
  return std::get<double>(value.data);
}

const std::string& Json::AsString(const Value& value)
{
  if (!IsString(value)) {
    throw TypeMismatchError(
        "Expected String, got " + std::to_string(static_cast<int>(GetType(value)))
    );
  }
  return std::get<std::string>(value.data);
}

const std::vector<Value>& Json::AsArray(const Value& value)
{
  if (!IsArray(value)) {
    throw TypeMismatchError("AsArray called on non-Array Value");
  }
  return std::get<std::vector<Value>>(value.data);
}

// -----------------------------------------------------------------------
// Array operations
// -----------------------------------------------------------------------

void Json::PushBack(Value& value, Value element)
{
  if (!IsArray(value)) {
    throw TypeMismatchError("PushBack called on non-Array Value");
  }
  std::get<std::vector<Value>>(value.data).push_back(std::move(element));
}

std::size_t Json::Size(const Value& value)
{
  if (IsArray(value)) {
    return std::get<std::vector<Value>>(value.data).size();
  }
  if (IsObject(value)) {
    return std::get<OrderedMap<Value>>(value.data).Size();
  }
  throw TypeMismatchError("Size called on non-Array, non-Object Value");
}

const Value& Json::At(const Value& value, std::size_t index)
{
  if (!IsArray(value)) {
    throw TypeMismatchError("At(index) called on non-Array Value");
  }
  const auto& elements = std::get<std::vector<Value>>(value.data);
  if (index >= elements.size()) {
    throw IndexOutOfRangeError(
        "Array index " + std::to_string(index) + " out of range (size " +
        std::to_string(elements.size()) + ")"
    );
  }
  return elements[index];
}

Value& Json::At(Value& value, std::size_t index)
{
  if (!IsArray(value)) {
    throw TypeMismatchError("At(index) called on non-Array Value");
  }
  auto& elements = std::get<std::vector<Value>>(value.data);
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

void Json::Insert(Value& value, std::string key, Value element)
{
  if (!IsObject(value)) {
    throw TypeMismatchError("Insert called on non-Object Value");
  }
  std::get<OrderedMap<Value>>(value.data).Insert(std::move(key), std::move(element));
}

bool Json::Contains(const Value& value, const std::string& key)
{
  if (!IsObject(value)) {
    throw TypeMismatchError("Contains called on non-Object Value");
  }
  return std::get<OrderedMap<Value>>(value.data).Contains(key);
}

const Value& Json::Get(const Value& value, const std::string& key)
{
  if (!IsObject(value)) {
    throw TypeMismatchError("Get called on non-Object Value");
  }
  return std::get<OrderedMap<Value>>(value.data).Get(key);
}

Value& Json::Get(Value& value, const std::string& key)
{
  if (!IsObject(value)) {
    throw TypeMismatchError("Get called on non-Object Value");
  }
  return std::get<OrderedMap<Value>>(value.data).Get(key);
}

const Value* Json::Find(const Value& value, const std::string& key)
{
  if (!IsObject(value)) {
    throw TypeMismatchError("Find called on non-Object Value");
  }
  return std::get<OrderedMap<Value>>(value.data).Find(key);
}

ObjectIterator Json::BeginObject(const Value& value)
{
  if (!IsObject(value)) {
    throw TypeMismatchError("BeginObject called on non-Object Value");
  }
  return ObjectIterator(std::get<OrderedMap<Value>>(value.data).GetEntries().begin());
}

ObjectIterator Json::EndObject(const Value& value)
{
  if (!IsObject(value)) {
    throw TypeMismatchError("EndObject called on non-Object Value");
  }
  return ObjectIterator(std::get<OrderedMap<Value>>(value.data).GetEntries().end());
}


Value Json::MakeArray()
{
  return Value{std::vector<Value>{}};
}

Value Json::MakeObject()
{
  return Value{OrderedMap<Value>{}};
}


} // namespace Amanuensis
  //
