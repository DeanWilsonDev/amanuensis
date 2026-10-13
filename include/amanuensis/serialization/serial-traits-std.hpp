
#pragma once

#include <amanuensis/serialization/serial-traits.hpp>
#include <amanuensis/serialization/serialize.hpp>

namespace amanuensis {

// -----------------------------------------------------------------------
// Built-in SerialTraits implementations (template bodies)
// -----------------------------------------------------------------------

template <typename ElementType>
core::Value
SerialTraits<std::vector<ElementType>>::ToValue(const std::vector<ElementType>& elements)
{
  core::Value arrayValue = core::Value::MakeArray();
  for (const auto& element : elements) {
    arrayValue.PushBack(amanuensis::ToValue<ElementType>(element));
  }
  return arrayValue;
}

template <typename ElementType>
std::vector<ElementType> SerialTraits<std::vector<ElementType>>::FromValue(const core::Value& value)
{
  const auto& rawArray = value.AsArray();
  std::vector<ElementType> result;
  result.reserve(rawArray.size());
  for (const auto& element : rawArray) {
    result.push_back(amanuensis::FromValue<ElementType>(element));
  }
  return result;
}

template <typename WrappedType>
core::Value
SerialTraits<std::optional<WrappedType>>::ToValue(const std::optional<WrappedType>& optionalValue)
{
  if (!optionalValue.has_value()) {
    return core::Value(); // null
  }
  return amanuensis::ToValue<WrappedType>(*optionalValue);
}

template <typename WrappedType>
std::optional<WrappedType>
SerialTraits<std::optional<WrappedType>>::FromValue(const core::Value& value)
{
  if (value.IsNull()) {
    return std::nullopt;
  }
  return amanuensis::FromValue<WrappedType>(value);
}

template <typename MappedType>
core::Value SerialTraits<std::map<std::string, MappedType>>::ToValue(
    const std::map<std::string, MappedType>& entries
)
{
  core::Value objectValue = core::Value::MakeObject();
  for (const auto& [key, mappedValue] : entries) {
    objectValue.Insert(key, amanuensis::ToValue<MappedType>(mappedValue));
  }
  return objectValue;
}

template <typename MappedType>
std::map<std::string, MappedType>
SerialTraits<std::map<std::string, MappedType>>::FromValue(const core::Value& value)
{
  std::map<std::string, MappedType> result;
  for (auto iterator = value.BeginObject(); iterator != value.EndObject(); ++iterator) {
    result[iterator->first] = amanuensis::FromValue<MappedType>(iterator->second);
  }
  return result;
}

} // namespace amanuensis
