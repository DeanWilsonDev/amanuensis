
#pragma once

#include <amanuensis/serialisation/json-traits.hpp>
#include <amanuensis/serialisation/serialize.hpp>

namespace Amanuensis {

// -----------------------------------------------------------------------
// Built-in JsonTraits implementations (template bodies)
// -----------------------------------------------------------------------

template <typename ElementType>
Value JsonTraits<std::vector<ElementType>>::ToJson(const std::vector<ElementType>& elements)
{
  Value arrayValue = Value::MakeArray();
  for (const auto& element : elements) {
    arrayValue.PushBack(Amanuensis::ToJson<ElementType>(element));
  }
  return arrayValue;
}

template <typename ElementType>
std::vector<ElementType> JsonTraits<std::vector<ElementType>>::FromJson(const Value& value)
{
  const auto& rawArray = value.AsArray();
  std::vector<ElementType> result;
  result.reserve(rawArray.size());
  for (const auto& element : rawArray) {
    result.push_back(Amanuensis::FromJson<ElementType>(element));
  }
  return result;
}

template <typename WrappedType>
Value JsonTraits<std::optional<WrappedType>>::ToJson(
    const std::optional<WrappedType>& optionalValue
)
{
  if (!optionalValue.has_value()) {
    return Value(); // null
  }
  return Amanuensis::ToJson<WrappedType>(*optionalValue);
}

template <typename WrappedType>
std::optional<WrappedType> JsonTraits<std::optional<WrappedType>>::FromJson(const Value& value)
{
  if (value.IsNull()) {
    return std::nullopt;
  }
  return Amanuensis::FromJson<WrappedType>(value);
}

template <typename MappedType>
Value JsonTraits<std::map<std::string, MappedType>>::ToJson(
    const std::map<std::string, MappedType>& entries
)
{
  Value objectValue = Value::MakeObject();
  for (const auto& [key, mappedValue] : entries) {
    objectValue.Insert(key, Amanuensis::ToJson<MappedType>(mappedValue));
  }
  return objectValue;
}

template <typename MappedType>
std::map<std::string, MappedType>
JsonTraits<std::map<std::string, MappedType>>::FromJson(const Value& value)
{
  std::map<std::string, MappedType> result;
  for (auto iterator = value.BeginObject(); iterator != value.EndObject(); ++iterator) {
    result[iterator->first] = Amanuensis::FromJson<MappedType>(iterator->second);
  }
  return result;
}

} // namespace Amanuensis
