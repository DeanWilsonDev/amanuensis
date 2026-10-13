#pragma once

#include <amanuensis/core/key-path.hpp>
#include <amanuensis/serialization/read-archive.hpp>
#include <amanuensis/serialization/serial-traits.hpp>
#include <amanuensis/serialization/serialize.hpp>

#include <map>
#include <string>
#include <vector>

namespace amanuensis {

// Declare a type atomic next to its AMANUENSIS_SERIALISABLE, in the type's own
// namespace:
//
//   AMANUENSIS_SERIALISABLE(Quaternion, x, y, z, w)
//   AMANUENSIS_ATOMIC(Quaternion)
#define AMANUENSIS_ATOMIC(TypeName)                                                                \
  [[maybe_unused]] constexpr bool AmanuensisIsAtomic(const TypeName*)                              \
  {                                                                                                \
    return true;                                                                                   \
  }

// Found by argument-dependent lookup, like AmanuensisSerialiseFree.
template <typename T>
inline constexpr bool IsAtomic = requires(const T* pointer) { AmanuensisIsAtomic(pointer); };

namespace detail {

template <typename T> struct IsStringMap : std::false_type {};
template <typename MappedType>
struct IsStringMap<std::map<std::string, MappedType>> : std::true_type {};

template <typename T>
void CollectAtomicPaths(const T& value, core::KeyPath& path, std::vector<core::KeyPath>& found);

// Handed to Serialise functions to walk a type's fields without building a
// Value.
class AtomicPathArchive {
public:
  AtomicPathArchive(core::KeyPath& path, std::vector<core::KeyPath>& found)
      : path_(path)
      , found_(found)
  {
  }

  template <typename FieldType> void Field(const char* key, const FieldType& fieldValue)
  {
    path_.push_back(key);
    CollectAtomicPaths(fieldValue, path_, found_);
    path_.pop_back();
  }

private:
  core::KeyPath& path_;
  std::vector<core::KeyPath>& found_;
};

// Follows ToValue's resolution order, so each path is where ToValue puts the
// object.
template <typename T>
void CollectAtomicPaths(const T& value, core::KeyPath& path, std::vector<core::KeyPath>& found)
{
  if constexpr (IsAtomic<T>) {
    found.push_back(path);
  }
  else if constexpr (IsOptional<T>::value) {
    if (value.has_value()) {
      CollectAtomicPaths(*value, path, found);
    }
  }
  else if constexpr (IsStringMap<T>::value) {
    for (const auto& [key, mappedValue] : value) {
      path.push_back(key);
      CollectAtomicPaths(mappedValue, path, found);
      path.pop_back();
    }
  }
  else if constexpr (HasSerialTraits<T>::value) {
    // Opaque: scalars, std::vector (arrays are marked whole anyway), and
    // SerialTraits specialisations, whose insides can't be seen.
  }
  else if constexpr (HasSerialiseMember<T>::value) {
    AtomicPathArchive archive(path, found);
    // const_cast is safe: AtomicPathArchive::Field only reads from fieldValue
    const_cast<T&>(value).Serialise(archive);
  }
  else if constexpr (HasSerialiseFree<T>::value) {
    AtomicPathArchive archive(path, found);
    AmanuensisSerialiseFree(const_cast<T&>(value), archive);
  }
}

} // namespace detail

// Paths go through struct fields, std::optional and std::map<std::string, T>.
// They never go into arrays, which are marked whole, or inside a SerialTraits
// specialisation.
template <typename T> std::vector<core::KeyPath> AtomicPaths(const T& value)
{
  std::vector<core::KeyPath> found;
  core::KeyPath path;
  detail::CollectAtomicPaths(value, path, found);
  return found;
}

} // namespace amanuensis
