#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace amanuensis::calamus {

// The '@' lines at the top of a Calamus file, kept apart from the entity's
// body. See docs/calamus-spec.md, "Header".
struct Header {
  // The entity type's data version, for migration. Not the Calamus syntax
  // version. Always at least 1.
  int version = 0;

  // The entity type, such as "Door".
  std::string type;

  // The entity's ID. A file without one gets one from the tooling on load.
  std::optional<std::uint64_t> id;

  // The prefab the entity is based on. The caller resolves it.
  std::optional<std::string> base;

  // core::StableHash of the canonical base the writer diffed against. A
  // hand-written file may not have one.
  std::optional<std::uint64_t> baseHash;
};

} // namespace amanuensis::calamus
