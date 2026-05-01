#include "amanuensis/value.hpp"
#include "ordered-map.hpp"

#include <utility>

namespace Amanuensis {

// -----------------------------------------------------------------------
// Value::Impl — the variant-based storage hidden behind the PIMPL.
// -----------------------------------------------------------------------

struct Value::Impl {
  using Storage = std::variant<
      std::nullptr_t,     // Null
      bool,               // Boolean
      long long,          // Integer
      double,             // Double
      std::string,        // String
      std::vector<Value>, // Array
      OrderedMap          // Object
      >;

  Storage storage;

  Impl()
      : storage(nullptr)
  {
  }
  explicit Impl(Storage s)
      : storage(std::move(s))
  {
  }
};

// -----------------------------------------------------------------------
// Variant index constants — must match the order in Impl::Storage.
// -----------------------------------------------------------------------

static constexpr std::size_t kNullIndex = 0;
static constexpr std::size_t kBoolIndex = 1;
static constexpr std::size_t kIntegerIndex = 2;
static constexpr std::size_t kDoubleIndex = 3;
static constexpr std::size_t kStringIndex = 4;
static constexpr std::size_t kArrayIndex = 5;
static constexpr std::size_t kObjectIndex = 6;

// -----------------------------------------------------------------------
// Construction
// -----------------------------------------------------------------------

Value::Value()
    : impl_(std::make_shared<Impl>())
{
}

Value::Value(bool booleanValue)
    : impl_(std::make_shared<Impl>(Impl::Storage{booleanValue}))
{
}

Value::Value(int integerValue)
    : impl_(std::make_shared<Impl>(Impl::Storage{static_cast<long long>(integerValue)}))
{
}

Value::Value(long long integerValue)
    : impl_(std::make_shared<Impl>(Impl::Storage{integerValue}))
{
}

Value::Value(double doubleValue)
    : impl_(std::make_shared<Impl>(Impl::Storage{doubleValue}))
{
}

Value::Value(const char* stringValue)
    : impl_(std::make_shared<Impl>(Impl::Storage{std::string(stringValue)}))
{
}

Value::Value(std::string stringValue)
    : impl_(std::make_shared<Impl>(Impl::Storage{std::move(stringValue)}))
{
}

Value Value::MakeArray()
{
  Value arrayValue;
  arrayValue.impl_ = std::make_shared<Impl>(Impl::Storage{std::vector<Value>{}});
  return arrayValue;
}

Value Value::MakeObject()
{
  Value objectValue;
  objectValue.impl_ = std::make_shared<Impl>(Impl::Storage{OrderedMap{}});
  return objectValue;
}

// -----------------------------------------------------------------------
// Type inspection
// -----------------------------------------------------------------------

ValueType Value::GetType() const
{
  switch (impl_->storage.index()) {
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
  return impl_->storage.index() == kNullIndex;
}
bool Value::IsBoolean() const
{
  return impl_->storage.index() == kBoolIndex;
}
bool Value::IsInteger() const
{
  return impl_->storage.index() == kIntegerIndex;
}
bool Value::IsDouble() const
{
  return impl_->storage.index() == kDoubleIndex;
}
bool Value::IsNumber() const
{
  return IsInteger() || IsDouble();
}
bool Value::IsString() const
{
  return impl_->storage.index() == kStringIndex;
}
bool Value::IsArray() const
{
  return impl_->storage.index() == kArrayIndex;
}
bool Value::IsObject() const
{
  return impl_->storage.index() == kObjectIndex;
}

// -----------------------------------------------------------------------
// Typed accessors — throw TypeMismatchError on wrong type
// -----------------------------------------------------------------------

bool Value::AsBoolean() const
{
  if (!IsBoolean()) {
    throw TypeMismatchError("Expected Boolean, got " + std::to_string(static_cast<int>(GetType())));
  }
  return std::get<bool>(impl_->storage);
}

long long Value::AsInteger() const
{
  if (!IsInteger()) {
    throw TypeMismatchError("Expected Integer, got " + std::to_string(static_cast<int>(GetType())));
  }
  return std::get<long long>(impl_->storage);
}

double Value::AsDouble() const
{
  if (!IsDouble()) {
    throw TypeMismatchError("Expected Double, got " + std::to_string(static_cast<int>(GetType())));
  }
  return std::get<double>(impl_->storage);
}

const std::string& Value::AsString() const
{
  if (!IsString()) {
    throw TypeMismatchError("Expected String, got " + std::to_string(static_cast<int>(GetType())));
  }
  return std::get<std::string>(impl_->storage);
}

// -----------------------------------------------------------------------
// Array operations
// -----------------------------------------------------------------------

void Value::PushBack(Value value)
{
  if (!IsArray()) {
    throw TypeMismatchError("PushBack called on non-Array Value");
  }
  std::get<std::vector<Value>>(impl_->storage).push_back(std::move(value));
}

std::size_t Value::Size() const
{
  if (IsArray()) {
    return std::get<std::vector<Value>>(impl_->storage).size();
  }
  if (IsObject()) {
    return std::get<OrderedMap>(impl_->storage).Size();
  }
  throw TypeMismatchError("Size called on non-Array, non-Object Value");
}

const Value& Value::At(std::size_t index) const
{
  if (!IsArray()) {
    throw TypeMismatchError("At(index) called on non-Array Value");
  }
  const auto& elements = std::get<std::vector<Value>>(impl_->storage);
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
  auto& elements = std::get<std::vector<Value>>(impl_->storage);
  if (index >= elements.size()) {
    throw IndexOutOfRangeError(
        "Array index " + std::to_string(index) + " out of range (size " +
        std::to_string(elements.size()) + ")"
    );
  }
  return elements[index];
}

const std::vector<Value>& Value::AsArray() const
{
  if (!IsArray()) {
    throw TypeMismatchError("AsArray called on non-Array Value");
  }
  return std::get<std::vector<Value>>(impl_->storage);
}

// -----------------------------------------------------------------------
// Object operations
// -----------------------------------------------------------------------

void Value::Insert(std::string key, Value value)
{
  if (!IsObject()) {
    throw TypeMismatchError("Insert called on non-Object Value");
  }
  std::get<OrderedMap>(impl_->storage).Insert(std::move(key), std::move(value));
}

bool Value::Contains(const std::string& key) const
{
  if (!IsObject()) {
    throw TypeMismatchError("Contains called on non-Object Value");
  }
  return std::get<OrderedMap>(impl_->storage).Contains(key);
}

const Value& Value::Get(const std::string& key) const
{
  if (!IsObject()) {
    throw TypeMismatchError("Get called on non-Object Value");
  }
  return std::get<OrderedMap>(impl_->storage).Get(key);
}

Value& Value::Get(const std::string& key)
{
  if (!IsObject()) {
    throw TypeMismatchError("Get called on non-Object Value");
  }
  return std::get<OrderedMap>(impl_->storage).Get(key);
}

const Value* Value::Find(const std::string& key) const
{
  if (!IsObject()) {
    throw TypeMismatchError("Find called on non-Object Value");
  }
  return std::get<OrderedMap>(impl_->storage).Find(key);
}

Value::ObjectIterator Value::BeginObject() const
{
  if (!IsObject()) {
    throw TypeMismatchError("BeginObject called on non-Object Value");
  }
  const auto& objectMap = std::get<OrderedMap>(impl_->storage);
  return ObjectIterator(objectMap.GetEntries().begin());
}

Value::ObjectIterator Value::EndObject() const
{
  if (!IsObject()) {
    throw TypeMismatchError("EndObject called on non-Object Value");
  }
  const auto& objectMap = std::get<OrderedMap>(impl_->storage);
  return ObjectIterator(objectMap.GetEntries().end());
}

} // namespace Amanuensis
