#include "amanuensis/core/value.hpp"
#include "amanuensis/json.hpp"
#include "amanuensis/core/ordered-map.hpp"
#include "amanuensis/core/object-iterator.hpp"
#include "amanuensis/core/errors.hpp"

#include <variant>

namespace amanuensis {

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

core::ValueType Json::GetType(const core::Value& value)
{
  switch (value.data.index()) {
  case kNullIndex:
    return core::ValueType::Null;
  case kBoolIndex:
    return core::ValueType::Boolean;
  case kIntegerIndex:
    return core::ValueType::Integer;
  case kDoubleIndex:
    return core::ValueType::Double;
  case kStringIndex:
    return core::ValueType::String;
  case kArrayIndex:
    return core::ValueType::Array;
  case kObjectIndex:
    return core::ValueType::Object;
  default:
    return core::ValueType::Null;
  }
}

bool Json::IsNull(const core::Value& value)
{
  return value.data.index() == kNullIndex;
}
bool Json::IsBoolean(const core::Value& value)
{
  return value.data.index() == kBoolIndex;
}
bool Json::IsInteger(const core::Value& value)
{
  return value.data.index() == kIntegerIndex;
}
bool Json::IsDouble(const core::Value& value)
{
  return value.data.index() == kDoubleIndex;
}
bool Json::IsNumber(const core::Value& value)
{
  return IsInteger(value) || IsDouble(value);
}
bool Json::IsString(const core::Value& value)
{
  return value.data.index() == kStringIndex;
}
bool Json::IsArray(const core::Value& value)
{
  return value.data.index() == kArrayIndex;
}
bool Json::IsObject(const core::Value& value)
{
  return value.data.index() == kObjectIndex;
}

// -----------------------------------------------------------------------
// Typed accessors
// -----------------------------------------------------------------------

bool Json::AsBoolean(const core::Value& value)
{
  if (!IsBoolean(value)) {
    throw core::TypeMismatchError(
        "Expected Boolean, got " + std::to_string(static_cast<int>(GetType(value)))
    );
  }
  return std::get<bool>(value.data);
}

long long Json::AsInteger(const core::Value& value)
{
  if (!IsInteger(value)) {
    throw core::TypeMismatchError(
        "Expected Integer, got " + std::to_string(static_cast<int>(GetType(value)))
    );
  }
  return std::get<long long>(value.data);
}

double Json::AsDouble(const core::Value& value)
{
  if (!IsDouble(value)) {
    throw core::TypeMismatchError(
        "Expected Double, got " + std::to_string(static_cast<int>(GetType(value)))
    );
  }
  return std::get<double>(value.data);
}

const std::string& Json::AsString(const core::Value& value)
{
  if (!IsString(value)) {
    throw core::TypeMismatchError(
        "Expected String, got " + std::to_string(static_cast<int>(GetType(value)))
    );
  }
  return std::get<std::string>(value.data);
}

const std::vector<core::Value>& Json::AsArray(const core::Value& value)
{
  if (!IsArray(value)) {
    throw core::TypeMismatchError("AsArray called on non-Array Value");
  }
  return std::get<std::vector<core::Value>>(value.data);
}

// -----------------------------------------------------------------------
// Array operations
// -----------------------------------------------------------------------

void Json::PushBack(core::Value& value, core::Value element)
{
  if (!IsArray(value)) {
    throw core::TypeMismatchError("PushBack called on non-Array Value");
  }
  std::get<std::vector<core::Value>>(value.data).push_back(std::move(element));
}

std::size_t Json::Size(const core::Value& value)
{
  if (IsArray(value)) {
    return std::get<std::vector<core::Value>>(value.data).size();
  }
  if (IsObject(value)) {
    return std::get<core::OrderedMap<core::Value>>(value.data).Size();
  }
  throw core::TypeMismatchError("Size called on non-Array, non-Object Value");
}

const core::Value& Json::At(const core::Value& value, std::size_t index)
{
  if (!IsArray(value)) {
    throw core::TypeMismatchError("At(index) called on non-Array Value");
  }
  const auto& elements = std::get<std::vector<core::Value>>(value.data);
  if (index >= elements.size()) {
    throw core::IndexOutOfRangeError(
        "Array index " + std::to_string(index) + " out of range (size " +
        std::to_string(elements.size()) + ")"
    );
  }
  return elements[index];
}

core::Value& Json::At(core::Value& value, std::size_t index)
{
  if (!IsArray(value)) {
    throw core::TypeMismatchError("At(index) called on non-Array Value");
  }
  auto& elements = std::get<std::vector<core::Value>>(value.data);
  if (index >= elements.size()) {
    throw core::IndexOutOfRangeError(
        "Array index " + std::to_string(index) + " out of range (size " +
        std::to_string(elements.size()) + ")"
    );
  }
  return elements[index];
}

// -----------------------------------------------------------------------
// Object operations
// -----------------------------------------------------------------------

void Json::Insert(core::Value& value, std::string key, core::Value element)
{
  if (!IsObject(value)) {
    throw core::TypeMismatchError("Insert called on non-Object Value");
  }
  std::get<core::OrderedMap<core::Value>>(value.data).Insert(std::move(key), std::move(element));
}

bool Json::Contains(const core::Value& value, const std::string& key)
{
  if (!IsObject(value)) {
    throw core::TypeMismatchError("Contains called on non-Object Value");
  }
  return std::get<core::OrderedMap<core::Value>>(value.data).Contains(key);
}

const core::Value& Json::Get(const core::Value& value, const std::string& key)
{
  if (!IsObject(value)) {
    throw core::TypeMismatchError("Get called on non-Object Value");
  }
  return std::get<core::OrderedMap<core::Value>>(value.data).Get(key);
}

core::Value& Json::Get(core::Value& value, const std::string& key)
{
  if (!IsObject(value)) {
    throw core::TypeMismatchError("Get called on non-Object Value");
  }
  return std::get<core::OrderedMap<core::Value>>(value.data).Get(key);
}

const core::Value* Json::Find(const core::Value& value, const std::string& key)
{
  if (!IsObject(value)) {
    throw core::TypeMismatchError("Find called on non-Object Value");
  }
  return std::get<core::OrderedMap<core::Value>>(value.data).Find(key);
}

core::ObjectIterator Json::BeginObject(const core::Value& value)
{
  if (!IsObject(value)) {
    throw core::TypeMismatchError("BeginObject called on non-Object Value");
  }
  return core::ObjectIterator(
      std::get<core::OrderedMap<core::Value>>(value.data).GetEntries().begin()
  );
}

core::ObjectIterator Json::EndObject(const core::Value& value)
{
  if (!IsObject(value)) {
    throw core::TypeMismatchError("EndObject called on non-Object Value");
  }
  return core::ObjectIterator(
      std::get<core::OrderedMap<core::Value>>(value.data).GetEntries().end()
  );
}

core::Value Json::MakeArray()
{
  return core::Value{std::vector<core::Value>{}};
}

core::Value Json::MakeObject()
{
  return core::Value{core::OrderedMap<core::Value>{}};
}

} // namespace amanuensis
  //
