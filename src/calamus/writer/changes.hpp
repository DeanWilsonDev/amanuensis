#pragma once

#include "amanuensis/core/key-path.hpp"
#include "amanuensis/core/value.hpp"

#include <set>
#include <vector>

namespace amanuensis::calamus {

struct Changes {
  // The paths Diff reported. Every line at or under one is marked.
  std::set<core::KeyPath> whole;
  // The paths with a reported path somewhere under them. Their block lines
  // are marked.
  std::set<core::KeyPath> containing;
};

Changes FindChanges(
    const core::Value& canonicalBase,
    const core::Value& canonicalEntity,
    const std::vector<core::KeyPath>& atomic
);

} // namespace amanuensis::calamus
