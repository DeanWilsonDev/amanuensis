#pragma once

#include <amanuensis/value.hpp>
#include <amanuensis/serialization/serial-traits.hpp>
#include <amanuensis/serialization/write-archive.hpp>
#include <amanuensis/serialization/read-archive.hpp>
#include <amanuensis/errors.hpp>

#include <optional>
#include <string>

namespace amanuensis {

namespace detail {

// Detect T::Serialise(Archive&) — intrusive member (Mechanism 2)
// We test with a dummy archive type.
struct ArchiveProbe {
  template <typename U> void Field(const char*, U&) {}
};

template <typename T, typename = void> struct HasSerialiseMember : std::false_type {};

template <typename T>
struct HasSerialiseMember<
    T,
    std::void_t<decltype(std::declval<T&>().Serialise(std::declval<ArchiveProbe&>()))>>
    : std::true_type {};

// Detect free Serialise(T&, Archive&) — what AMANUENSIS_SERIALISABLE generates (Mechanism 1)
template <typename T, typename = void> struct HasSerialiseFree : std::false_type {};

template <typename T>
struct HasSerialiseFree<
    T,
    std::void_t<
        decltype(AmanuensisSerialiseFree(std::declval<T&>(), std::declval<ArchiveProbe&>()))>>
    : std::true_type {};
} // namespace detail

// -----------------------------------------------------------------------
// Core ToValue / FromValue — resolution order:
//   1. SerialTraits<T> specialisation
//   2. Intrusive Serialise member
//   3. Free AmanuensisSerialiseFree function (macro-generated)
//   4. Compile error
// -----------------------------------------------------------------------

template <typename T> Value ToValue(const T& value)
{
  if constexpr (HasSerialTraits<T>::value) {
    return SerialTraits<T>::ToValue(value);
  }
  else if constexpr (detail::HasSerialiseMember<T>::value) {
    WriteArchive archive;
    // const_cast is safe: WriteArchive::Field only reads from fieldValue
    const_cast<T&>(value).Serialise(archive);
    return archive.GetValue();
  }
  else if constexpr (detail::HasSerialiseFree<T>::value) {
    WriteArchive archive;
    AmanuensisSerialiseFree(const_cast<T&>(value), archive);
    return archive.GetValue();
  }
  else {
    static_assert(
        HasSerialTraits<T>::value, "Type has no Amanuensis serialisation. Opt in via: "
                                   "(1) AMANUENSIS_SERIALISABLE macro, "
                                   "(2) intrusive Serialise member, or "
                                   "(3) SerialTraits<T> specialisation."
    );
  }
}

template <typename T> T FromValue(const Value& value)
{
  if constexpr (HasSerialTraits<T>::value) {
    return SerialTraits<T>::FromValue(value);
  }
  else if constexpr (detail::HasSerialiseMember<T>::value) {
    T result{};
    ReadArchive archive(value);
    result.Serialise(archive);
    return result;
  }
  else if constexpr (detail::HasSerialiseFree<T>::value) {
    T result{};
    ReadArchive archive(value);
    AmanuensisSerialiseFree(result, archive);
    return result;
  }
  else {
    static_assert(
        HasSerialTraits<T>::value, "Type has no Amanuensis deserialisation. Opt in via: "
                                   "(1) AMANUENSIS_SERIALISABLE macro, "
                                   "(2) intrusive Serialise member, or "
                                   "(3) SerialTraits<T> specialisation."
    );
  }
}

// -----------------------------------------------------------------------
// TryFromValue — non-throwing variant
// -----------------------------------------------------------------------

template <typename T> struct FromValueResult {
  bool succeeded;
  T value;
  std::string errorMessage;
};

template <typename T> FromValueResult<T> TryFromValue(const Value& jsonValue)
{
  try {
    T result = FromValue<T>(jsonValue);
    return FromValueResult<T>{true, std::move(result), {}};
  }
  catch (const std::exception& error) {
    return FromValueResult<T>{false, T{}, error.what()};
  }
}

#ifndef AMANUENSIS_NO_COMPAT
// Old names, kept until the consumer sweep (see compat.hpp).
template <typename T> using FromJsonResult = FromValueResult<T>;

template <typename T> Value ToJson(const T& value)
{
  return ToValue<T>(value);
}
template <typename T> T FromJson(const Value& value)
{
  return FromValue<T>(value);
}
template <typename T> FromValueResult<T> TryFromJson(const Value& value)
{
  return TryFromValue<T>(value);
}
#endif

// -----------------------------------------------------------------------
// ReadArchive::Field — deserialise a single field from the source object
// -----------------------------------------------------------------------

template <typename FieldType> void ReadArchive::Field(const char* jsonKey, FieldType& fieldValue)
{
  // std::optional fields: missing or null key → leave empty
  if constexpr (detail::IsOptional<FieldType>::value) {
    const Value* found = Json::Find(source_, jsonKey);
    if (found == nullptr || Json::IsNull(*found)) {
      fieldValue = std::nullopt;
      return;
    }
    fieldValue = FromValue<typename FieldType::value_type>(*found);
  }
  else {
    const Value* found = Json::Find(source_, jsonKey);
    if (found == nullptr) {
      throw KeyNotFoundError(std::string("Missing required field: \"") + jsonKey + "\"");
    }
    fieldValue = FromValue<FieldType>(*found);
  }
}

// -----------------------------------------------------------------------
// WriteArchive::Field — serialise a single field into the object
// -----------------------------------------------------------------------

template <typename FieldType>
void WriteArchive::Field(const char* jsonKey, const FieldType& fieldValue)
{
  Json::Insert(object_, std::string(jsonKey), ToValue<FieldType>(fieldValue));
}

} // namespace amanuensis
