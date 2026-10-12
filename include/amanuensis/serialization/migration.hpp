#pragma once

#include <amanuensis/core/migrate.hpp>
#include <amanuensis/core/value.hpp>

namespace amanuensis {

// -----------------------------------------------------------------------
// Data versions and migration
//
// A type's data version goes up when its layout changes, such as a renamed
// field or a new unit. Declare the current version next to
// AMANUENSIS_SERIALISABLE, in the type's own namespace, with a function that
// upgrades data by one version, found by ADL like AMANUENSIS_SERIALISABLE's:
//
//   AMANUENSIS_SERIALISABLE(Door, position, angle);
//   AMANUENSIS_VERSION(Door, 3);
//
//   // Called once per step: fromVersion -> fromVersion + 1.
//   void AmanuensisMigrate(Door*, amanuensis::core::Value& value, int fromVersion)
//   {
//     if (fromVersion == 1 && value.Contains("pos")) {
//       value.Insert("position", value.Get("pos"));
//       value.Erase("pos");
//     }
//     ...
//   }
//
// A type without AMANUENSIS_VERSION is at version 1. A step may be given only
// part of the data, such as a Calamus file's overrides, so it should cope
// with missing keys.
// -----------------------------------------------------------------------

#define AMANUENSIS_VERSION(TypeName, number)                                                       \
  [[maybe_unused]] constexpr int AmanuensisVersion(const TypeName*)                                \
  {                                                                                                \
    return number;                                                                                 \
  }

namespace detail {

template <typename T> constexpr int VersionOf()
{
  if constexpr (requires(const T* pointer) { AmanuensisVersion(pointer); }) {
    constexpr int version = AmanuensisVersion(static_cast<const T*>(nullptr));
    static_assert(version >= 1, "AMANUENSIS_VERSION must be at least 1");
    return version;
  }
  else {
    return 1;
  }
}

template <typename T>
concept HasMigrate = requires(T* pointer, core::Value& value, int fromVersion) {
  AmanuensisMigrate(pointer, value, fromVersion);
};

} // namespace detail

// T's current data version: its AMANUENSIS_VERSION, or 1.
template <typename T> inline constexpr int CurrentVersion = detail::VersionOf<T>();

// Upgrades value by one version of T, through T's AmanuensisMigrate.
template <typename T> void MigrateStep(core::Value& value, int fromVersion)
{
  if constexpr (detail::HasMigrate<T>) {
    AmanuensisMigrate(static_cast<T*>(nullptr), value, fromVersion);
  }
  else {
    static_assert(
        CurrentVersion<T> == 1,
        "A type with AMANUENSIS_VERSION above 1 needs an AmanuensisMigrate function"
    );
  }
}

// Brings value, which holds T's data at fromVersion, up to T's current
// version. Never throws: a newer version or a failing step comes back as an
// error.
template <typename T> core::MigrateResult Migrate(core::Value& value, int fromVersion)
{
  return core::Migrate(value, fromVersion, CurrentVersion<T>, MigrateStep<T>);
}

} // namespace amanuensis
