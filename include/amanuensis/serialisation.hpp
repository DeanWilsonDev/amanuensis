#pragma once

#include "value.hpp"

#include <optional>
#include <string>
#include <type_traits>
#include <vector>
#include <map>

namespace Amanuensis {

// -----------------------------------------------------------------------
// Forward declarations
// -----------------------------------------------------------------------

template <typename T> struct JsonTraits; // Mechanism 3 — specialise for external types

// -----------------------------------------------------------------------
// Detection helpers — used by the resolution-order logic to pick the
// right mechanism at compile time.
// -----------------------------------------------------------------------

namespace Detail {

// Detect JsonTraits<T>::ToJson
template <typename T, typename = void> struct HasJsonTraits : std::false_type {};

template <typename T>
struct HasJsonTraits<T, std::void_t<decltype(JsonTraits<T>::ToJson(std::declval<const T&>()))>>
    : std::true_type {};

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
// WriteArchive — handed to Serialise functions when converting T → Value
// -----------------------------------------------------------------------

class WriteArchive {
public:
  WriteArchive()
      : object_(Value::MakeObject())
  {
  }

  template <typename FieldType> void Field(const char* jsonKey, const FieldType& fieldValue);

  Value& GetValue() { return object_; }

private:
  Value object_;
};

// -----------------------------------------------------------------------
// ReadArchive — handed to Serialise functions when converting Value → T
// -----------------------------------------------------------------------

class ReadArchive {
public:
  explicit ReadArchive(const Value& source)
      : source_(source)
  {
  }

  template <typename FieldType> void Field(const char* jsonKey, FieldType& fieldValue);

private:
  const Value& source_;
};

// -----------------------------------------------------------------------
// Built-in JsonTraits for fundamental and standard-library types
// -----------------------------------------------------------------------

// bool
template <> struct JsonTraits<bool> {
  static Value ToJson(const bool& value) { return Value(value); }
  static bool FromJson(const Value& value) { return value.AsBoolean(); }
};

// int
template <> struct JsonTraits<int> {
  static Value ToJson(const int& value) { return Value(value); }
  static int FromJson(const Value& value) { return static_cast<int>(value.AsInteger()); }
};

// long long
template <> struct JsonTraits<long long> {
  static Value ToJson(const long long& value) { return Value(value); }
  static long long FromJson(const Value& value) { return value.AsInteger(); }
};

// double
template <> struct JsonTraits<double> {
  static Value ToJson(const double& value) { return Value(value); }
  static double FromJson(const Value& value) { return value.AsDouble(); }
};

// std::string
template <> struct JsonTraits<std::string> {
  static Value ToJson(const std::string& value) { return Value(value); }
  static std::string FromJson(const Value& value) { return value.AsString(); }
};

// std::vector<T>
template <typename ElementType> struct JsonTraits<std::vector<ElementType>> {
  static Value ToJson(const std::vector<ElementType>& elements);
  static std::vector<ElementType> FromJson(const Value& value);
};

// std::optional<T>
template <typename WrappedType> struct JsonTraits<std::optional<WrappedType>> {
  static Value ToJson(const std::optional<WrappedType>& optionalValue);
  static std::optional<WrappedType> FromJson(const Value& value);
};

// std::map<std::string, T>
template <typename MappedType> struct JsonTraits<std::map<std::string, MappedType>> {
  static Value ToJson(const std::map<std::string, MappedType>& entries);
  static std::map<std::string, MappedType> FromJson(const Value& value);
};

// -----------------------------------------------------------------------
// Core ToJson / FromJson — resolution order:
//   1. JsonTraits<T> specialisation
//   2. Intrusive Serialise member
//   3. Free AmanuensisSerialiseFree function (macro-generated)
//   4. Compile error
// -----------------------------------------------------------------------

template <typename T> Value ToJson(const T& value)
{
  if constexpr (Detail::HasJsonTraits<T>::value) {
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
        Detail::HasJsonTraits<T>::value, "Type has no Amanuensis serialisation. Opt in via: "
                                         "(1) AMANUENSIS_SERIALISABLE macro, "
                                         "(2) intrusive Serialise member, or "
                                         "(3) JsonTraits<T> specialisation."
    );
  }
}

template <typename T> T FromJson(const Value& value)
{
  if constexpr (Detail::HasJsonTraits<T>::value) {
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
        Detail::HasJsonTraits<T>::value, "Type has no Amanuensis deserialisation. Opt in via: "
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
// WriteArchive::Field — serialise a single field into the object
// -----------------------------------------------------------------------

template <typename FieldType>
void WriteArchive::Field(const char* jsonKey, const FieldType& fieldValue)
{
  object_.Insert(std::string(jsonKey), ToJson<FieldType>(fieldValue));
}

// -----------------------------------------------------------------------
// ReadArchive::Field — deserialise a single field from the source object
// -----------------------------------------------------------------------

namespace Detail {
template <typename T> struct IsOptional : std::false_type {};
template <typename T> struct IsOptional<std::optional<T>> : std::true_type {};
} // namespace Detail

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

// -----------------------------------------------------------------------
// AMANUENSIS_SERIALISABLE macro — Mechanism 1 (the default path)
//
// Expands to a free function that the resolution-order logic can find
// via ADL.  The function visits each named field using the C++ field
// name as the JSON key.
// -----------------------------------------------------------------------

#define AMANUENSIS_DETAIL_FIELD(archive, field) archive.Field(#field, instance.field);

#define AMANUENSIS_SERIALISABLE(TypeName, ...)                                                     \
  template <typename ArchiveType>                                                                  \
  void AmanuensisSerialiseFree(TypeName& instance, ArchiveType& archive)                           \
  {                                                                                                \
    AMANUENSIS_DETAIL_APPLY(archive, __VA_ARGS__)                                                  \
  }

// Helper: apply AMANUENSIS_DETAIL_FIELD to each argument via a FOR_EACH pattern.
// We support up to 32 fields — more than enough for any real struct.

#define AMANUENSIS_DETAIL_APPLY(archive, ...) AMANUENSIS_DETAIL_FOR_EACH(archive, __VA_ARGS__)

// Counting / dispatching macros
#define AMANUENSIS_DETAIL_FOR_EACH(archive, ...)                                                                                                                                                                                                                    \
  AMANUENSIS_DETAIL_EXPAND(                                                                                                                                                                                                                                         \
      AMANUENSIS_DETAIL_SELECT(__VA_ARGS__, FE_32, FE_31, FE_30, FE_29, FE_28, FE_27, FE_26, FE_25, FE_24, FE_23, FE_22, FE_21, FE_20, FE_19, FE_18, FE_17, FE_16, FE_15, FE_14, FE_13, FE_12, FE_11, FE_10, FE_9, FE_8, FE_7, FE_6, FE_5, FE_4, FE_3, FE_2, FE_1)( \
          archive, __VA_ARGS__                                                                                                                                                                                                                                      \
      )                                                                                                                                                                                                                                                             \
  )

#define AMANUENSIS_DETAIL_EXPAND(x) x

#define AMANUENSIS_DETAIL_SELECT(                                                                  \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,     \
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, NAME, ...                          \
)                                                                                                  \
  NAME

#define FE_1(ar, a) ar.Field(#a, instance.a);
#define FE_2(ar, a, ...)                                                                           \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_1(ar, __VA_ARGS__))
#define FE_3(ar, a, ...)                                                                           \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_2(ar, __VA_ARGS__))
#define FE_4(ar, a, ...)                                                                           \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_3(ar, __VA_ARGS__))
#define FE_5(ar, a, ...)                                                                           \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_4(ar, __VA_ARGS__))
#define FE_6(ar, a, ...)                                                                           \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_5(ar, __VA_ARGS__))
#define FE_7(ar, a, ...)                                                                           \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_6(ar, __VA_ARGS__))
#define FE_8(ar, a, ...)                                                                           \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_7(ar, __VA_ARGS__))
#define FE_9(ar, a, ...)                                                                           \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_8(ar, __VA_ARGS__))
#define FE_10(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_9(ar, __VA_ARGS__))
#define FE_11(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_10(ar, __VA_ARGS__))
#define FE_12(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_11(ar, __VA_ARGS__))
#define FE_13(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_12(ar, __VA_ARGS__))
#define FE_14(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_13(ar, __VA_ARGS__))
#define FE_15(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_14(ar, __VA_ARGS__))
#define FE_16(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_15(ar, __VA_ARGS__))
#define FE_17(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_16(ar, __VA_ARGS__))
#define FE_18(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_17(ar, __VA_ARGS__))
#define FE_19(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_18(ar, __VA_ARGS__))
#define FE_20(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_19(ar, __VA_ARGS__))
#define FE_21(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_20(ar, __VA_ARGS__))
#define FE_22(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_21(ar, __VA_ARGS__))
#define FE_23(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_22(ar, __VA_ARGS__))
#define FE_24(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_23(ar, __VA_ARGS__))
#define FE_25(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_24(ar, __VA_ARGS__))
#define FE_26(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_25(ar, __VA_ARGS__))
#define FE_27(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_26(ar, __VA_ARGS__))
#define FE_28(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_27(ar, __VA_ARGS__))
#define FE_29(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_28(ar, __VA_ARGS__))
#define FE_30(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_29(ar, __VA_ARGS__))
#define FE_31(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_30(ar, __VA_ARGS__))
#define FE_32(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_31(ar, __VA_ARGS__))

} // namespace Amanuensis
