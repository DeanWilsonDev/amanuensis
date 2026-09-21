#include "amanuensis/json-value.hpp"
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

JsonValueType Json::GetType(const JsonValue& value)
{
  switch (value.data.index()) {
  case kNullIndex:
    return JsonValueType::Null;
  case kBoolIndex:
    return JsonValueType::Boolean;
  case kIntegerIndex:
    return JsonValueType::Integer;
  case kDoubleIndex:
    return JsonValueType::Double;
  case kStringIndex:
    return JsonValueType::String;
  case kArrayIndex:
    return JsonValueType::Array;
  case kObjectIndex:
    return JsonValueType::Object;
  default:
    return JsonValueType::Null;
  }
}

bool Json::IsNull(const JsonValue& value)
{
  return value.data.index() == kNullIndex;
}
bool Json::IsBoolean(const JsonValue& value)
{
  return value.data.index() == kBoolIndex;
}
bool Json::IsInteger(const JsonValue& value)
{
  return value.data.index() == kIntegerIndex;
}
bool Json::IsDouble(const JsonValue& value)
{
  return value.data.index() == kDoubleIndex;
}
bool Json::IsNumber(const JsonValue& value)
{
  return IsInteger(value) || IsDouble(value);
}
bool Json::IsString(const JsonValue& value)
{
  return value.data.index() == kStringIndex;
}
bool Json::IsArray(const JsonValue& value)
{
  return value.data.index() == kArrayIndex;
}
bool Json::IsObject(const JsonValue& value)
{
  return value.data.index() == kObjectIndex;
}

// -----------------------------------------------------------------------
// Typed accessors
// -----------------------------------------------------------------------

bool Json::AsBoolean(const JsonValue& value)
{
  if (!IsBoolean(value)) {
    throw TypeMismatchError(
        "Expected Boolean, got " + std::to_string(static_cast<int>(GetType(value)))
    );
  }
  return std::get<bool>(value.data);
}

long long Json::AsInteger(const JsonValue& value)
{
  if (!IsInteger(value)) {
    throw TypeMismatchError(
        "Expected Integer, got " + std::to_string(static_cast<int>(GetType(value)))
    );
  }
  return std::get<long long>(value.data);
}

double Json::AsDouble(const JsonValue& value)
{
  if (!IsDouble(value)) {
    throw TypeMismatchError(
        "Expected Double, got " + std::to_string(static_cast<int>(GetType(value)))
    );
  }
  return std::get<double>(value.data);
}

const std::string& Json::AsString(const JsonValue& value)
{
  if (!IsString(value)) {
    throw TypeMismatchError(
        "Expected String, got " + std::to_string(static_cast<int>(GetType(value)))
    );
  }
  return std::get<std::string>(value.data);
}

const std::vector<JsonValue>& Json::AsArray(const JsonValue& value)
{
  if (!IsArray(value)) {
    throw TypeMismatchError("AsArray called on non-Array JsonValue");
  }
  return std::get<std::vector<JsonValue>>(value.data);
}

// -----------------------------------------------------------------------
// Array operations
// -----------------------------------------------------------------------

void Json::PushBack(JsonValue& value, JsonValue element)
{
  if (!IsArray(value)) {
    throw TypeMismatchError("PushBack called on non-Array JsonValue");
  }
  std::get<std::vector<JsonValue>>(value.data).push_back(std::move(element));
}

std::size_t Json::Size(const JsonValue& value)
{
  if (IsArray(value)) {
    return std::get<std::vector<JsonValue>>(value.data).size();
  }
  if (IsObject(value)) {
    return std::get<OrderedMap<JsonValue>>(value.data).Size();
  }
  throw TypeMismatchError("Size called on non-Array, non-Object JsonValue");
}

const JsonValue& Json::At(const JsonValue& value, std::size_t index)
{
  if (!IsArray(value)) {
    throw TypeMismatchError("At(index) called on non-Array JsonValue");
  }
  const auto& elements = std::get<std::vector<JsonValue>>(value.data);
  if (index >= elements.size()) {
    throw IndexOutOfRangeError(
        "Array index " + std::to_string(index) + " out of range (size " +
        std::to_string(elements.size()) + ")"
    );
  }
  return elements[index];
}

JsonValue& Json::At(JsonValue& value, std::size_t index)
{
  if (!IsArray(value)) {
    throw TypeMismatchError("At(index) called on non-Array JsonValue");
  }
  auto& elements = std::get<std::vector<JsonValue>>(value.data);
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

void Json::Insert(JsonValue& value, std::string key, JsonValue element)
{
  if (!IsObject(value)) {
    throw TypeMismatchError("Insert called on non-Object JsonValue");
  }
  std::get<OrderedMap<JsonValue>>(value.data).Insert(std::move(key), std::move(element));
}

bool Json::Contains(const JsonValue& value, const std::string& key)
{
  if (!IsObject(value)) {
    throw TypeMismatchError("Contains called on non-Object JsonValue");
  }
  return std::get<OrderedMap<JsonValue>>(value.data).Contains(key);
}

const JsonValue& Json::Get(const JsonValue& value, const std::string& key)
{
  if (!IsObject(value)) {
    throw TypeMismatchError("Get called on non-Object JsonValue");
  }
  return std::get<OrderedMap<JsonValue>>(value.data).Get(key);
}

JsonValue& Json::Get(JsonValue& value, const std::string& key)
{
  if (!IsObject(value)) {
    throw TypeMismatchError("Get called on non-Object JsonValue");
  }
  return std::get<OrderedMap<JsonValue>>(value.data).Get(key);
}

const JsonValue* Json::Find(const JsonValue& value, const std::string& key)
{
  if (!IsObject(value)) {
    throw TypeMismatchError("Find called on non-Object JsonValue");
  }
  return std::get<OrderedMap<JsonValue>>(value.data).Find(key);
}

ObjectIterator Json::BeginObject(const JsonValue& value)
{
  if (!IsObject(value)) {
    throw TypeMismatchError("BeginObject called on non-Object JsonValue");
  }
  return ObjectIterator(std::get<OrderedMap<JsonValue>>(value.data).GetEntries().begin());
}

ObjectIterator Json::EndObject(const JsonValue& value)
{
  if (!IsObject(value)) {
    throw TypeMismatchError("EndObject called on non-Object JsonValue");
  }
  return ObjectIterator(std::get<OrderedMap<JsonValue>>(value.data).GetEntries().end());
}

JsonValue Json::MakeArray()
{
  return JsonValue{std::vector<JsonValue>{}};
}

JsonValue Json::MakeObject()
{
  return JsonValue{OrderedMap<JsonValue>{}};
}

} // namespace Amanuensis
  //
