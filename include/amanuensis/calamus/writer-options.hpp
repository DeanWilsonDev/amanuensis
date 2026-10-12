#pragma once

#include "amanuensis/core/key-path.hpp"

#include <vector>

namespace amanuensis::calamus {

struct WriterOptions {
  // The paths of the entity's atomic objects, such as quaternions and
  // colours. When any line inside one is marked, the writer marks every line
  // in it, so a change to the base can't combine new and old components.
  // amanuensis::AtomicPaths(entity) lists them for types declared
  // AMANUENSIS_ATOMIC. A path where the entity has no Object is ignored.
  std::vector<core::KeyPath> atomic;
};

} // namespace amanuensis::calamus
