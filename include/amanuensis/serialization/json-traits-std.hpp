
#pragma once

#include <amanuensis/serialization/json-traits.hpp>
#include <amanuensis/serialization/serialize.hpp>

namespace Amanuensis {

// -----------------------------------------------------------------------
// Built-in JsonTraits implementations (template bodies)
// -----------------------------------------------------------------------

template <typename ElementType>
JsonValue JsonTraits<std::vector<ElementType>>::ToJson(const std::vector<ElementType>& elements)
{
  JsonValue arrayJsonValue = Json::MakeArray();
  for (const auto& element : elements) {
    Json::PushBack(arrayJsonValue, Amanuensis::ToJson<ElementType>(element));
  }
  return arrayJsonValue;
}

template <typename ElementType>
std::vector<ElementType> JsonTraits<std::vector<ElementType>>::FromJson(const JsonValue& value)
{
  const auto& rawArray = Json::AsArray(value);
  std::vector<ElementType> result;
  result.reserve(rawArray.size());
  for (const auto& element : rawArray) {
    result.push_back(Amanuensis::FromJson<ElementType>(element));
  }
  return result;
}

template <typename WrappedType>
JsonValue JsonTraits<std::optional<WrappedType>>::ToJson(
    const std::optional<WrappedType>& optionalJsonValue
)
{
  if (!optionalJsonValue.has_value()) {
    return JsonValue(); // null
  }
  return Amanuensis::ToJson<WrappedType>(*optionalJsonValue);
}

template <typename WrappedType>
std::optional<WrappedType> JsonTraits<std::optional<WrappedType>>::FromJson(const JsonValue& value)
{
  if (Json::IsNull(value)) {
    return std::nullopt;
  }
  return Amanuensis::FromJson<WrappedType>(value);
}

template <typename MappedType>
JsonValue JsonTraits<std::map<std::string, MappedType>>::ToJson(
    const std::map<std::string, MappedType>& entries
)
{
  JsonValue objectJsonValue = Json::MakeObject();
  for (const auto& [key, mappedJsonValue] : entries) {
    Json::Insert(objectJsonValue, key, Amanuensis::ToJson<MappedType>(mappedJsonValue));
  }
  return objectJsonValue;
}

template <typename MappedType>
std::map<std::string, MappedType>
JsonTraits<std::map<std::string, MappedType>>::FromJson(const JsonValue& value)
{
  std::map<std::string, MappedType> result;
  for (auto iterator = Json::BeginObject(value); iterator != Json::EndObject(value); ++iterator) {
    result[iterator->first] = Amanuensis::FromJson<MappedType>(iterator->second);
  }
  return result;
}

} // namespace Amanuensis
