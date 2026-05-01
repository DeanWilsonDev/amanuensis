#pragma once

#include <amanuensis/value.hpp>
#include <amanuensis/serialisation/json-traits.hpp>
#include <amanuensis/serialisation/write-archive.hpp>
#include <amanuensis/serialisation/read-archive.hpp>

#include <string>

namespace Amanuensis {

namespace Detail {

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
} // namespace Detail

// -----------------------------------------------------------------------
// Core ToJson / FromJson — resolution order:
//   1. JsonTraits<T> specialisation
//   2. Intrusive Serialise member
//   3. Free AmanuensisSerialiseFree function (macro-generated)
//   4. Compile error
// -----------------------------------------------------------------------

template <typename T> Value ToJson(const T& value)
{
  if constexpr (HasJsonTraits<T>::value) {
    return JsonTraits<T>::ToJson(value);
  }
  else if constexpr (Detail::HasSerialiseMember<T>::value) {
    WriteArchive archive;
    // const_cast is safe: WriteArchive::Field only reads from fieldValue
    const_cast<T&>(value).Serialise(archive);
    return archive.GetValue();
  }
  else if constexpr (Detail::HasSerialiseFree<T>::value) {
    WriteArchive archive;
    AmanuensisSerialiseFree(const_cast<T&>(value), archive);
    return archive.GetValue();
  }
  else {
    static_assert(
        HasJsonTraits<T>::value, "Type has no Amanuensis serialisation. Opt in via: "
                                 "(1) AMANUENSIS_SERIALISABLE macro, "
                                 "(2) intrusive Serialise member, or "
                                 "(3) JsonTraits<T> specialisation."
    );
  }
}

template <typename T> T FromJson(const Value& value)
{
  if constexpr (HasJsonTraits<T>::value) {
    return JsonTraits<T>::FromJson(value);
  }
  else if constexpr (Detail::HasSerialiseMember<T>::value) {
    T result{};
    ReadArchive archive(value);
    result.Serialise(archive);
    return result;
  }
  else if constexpr (Detail::HasSerialiseFree<T>::value) {
    T result{};
    ReadArchive archive(value);
    AmanuensisSerialiseFree(result, archive);
    return result;
  }
  else {
    static_assert(
        HasJsonTraits<T>::value, "Type has no Amanuensis deserialisation. Opt in via: "
                                 "(1) AMANUENSIS_SERIALISABLE macro, "
                                 "(2) intrusive Serialise member, or "
                                 "(3) JsonTraits<T> specialisation."
    );
  }
}

// -----------------------------------------------------------------------
// TryFromJson — non-throwing variant
// -----------------------------------------------------------------------

template <typename T> struct FromJsonResult {
  bool succeeded;
  T value;
  std::string errorMessage;
};

template <typename T> FromJsonResult<T> TryFromJson(const Value& jsonValue)
{
  try {
    T result = FromJson<T>(jsonValue);
    return FromJsonResult<T>{true, std::move(result), {}};
  }
  catch (const std::exception& error) {
    return FromJsonResult<T>{false, T{}, error.what()};
  }
}


// -----------------------------------------------------------------------
// ReadArchive::Field — deserialise a single field from the source object
// -----------------------------------------------------------------------

template <typename FieldType> void ReadArchive::Field(const char* jsonKey, FieldType& fieldValue)
{
  // std::optional fields: missing or null key → leave empty
  if constexpr (Detail::IsOptional<FieldType>::value) {
    const Value* found = source_.Find(jsonKey);
    if (found == nullptr || found->IsNull()) {
      fieldValue = std::nullopt;
      return;
    }
    fieldValue = FromJson<typename FieldType::value_type>(*found);
  }
  else {
    const Value* found = source_.Find(jsonKey);
    if (found == nullptr) {
      throw KeyNotFoundError(std::string("Missing required field: \"") + jsonKey + "\"");
    }
    fieldValue = FromJson<FieldType>(*found);
  }
}


// -----------------------------------------------------------------------
// WriteArchive::Field — serialise a single field into the object
// -----------------------------------------------------------------------

template <typename FieldType>
void WriteArchive::Field(const char* jsonKey, const FieldType& fieldValue)
{
  object_.Insert(std::string(jsonKey), ToJson<FieldType>(fieldValue));
}

} // namespace Amanuensis
