#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace amanuensis::calamus {

struct Header {
  // The entity type's data version, not the Calamus syntax version.
  int version = 0;

  std::string type;

  std::optional<std::uint64_t> id;

  std::optional<std::string> base;

  // core::StableHash of the canonical base the writer diffed against.
  std::optional<std::uint64_t> baseHash;
};

} // namespace amanuensis::calamus
