#pragma once

#include "amanuensis/core/value.hpp"
#include <optional>

namespace amanuensis {

namespace detail {
template <typename T> struct IsOptional : std::false_type {};
template <typename T> struct IsOptional<std::optional<T>> : std::true_type {};
} // namespace detail

// -----------------------------------------------------------------------
// ReadArchive — handed to Serialise functions when converting Value → T
// -----------------------------------------------------------------------

class ReadArchive {
public:
  explicit ReadArchive(const core::Value& source)
      : source_(source)
  {
  }

  template <typename FieldType> void Field(const char* jsonKey, FieldType& fieldValue);

private:
  const core::Value& source_;
};

} // namespace amanuensis
