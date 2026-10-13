#pragma once

#include "amanuensis/core/key-path.hpp"

#include <vector>

namespace amanuensis::calamus {

struct WriterOptions {
  // When any line inside one of these objects is marked, the writer marks every
  // line in it, so a change to the base can't combine new and old components.
  // amanuensis::AtomicPaths(entity) lists them for types declared
  // AMANUENSIS_ATOMIC.
  std::vector<core::KeyPath> atomic;
};

} // namespace amanuensis::calamus
