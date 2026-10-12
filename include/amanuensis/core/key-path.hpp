#pragma once
#include "amanuensis/compat.hpp"

#include <string>
#include <vector>

namespace amanuensis::core {

// Keys are kept whole because they can hold any character, including dots.
// A path never indexes into an array: arrays compare and override as a whole.
using KeyPath = std::vector<std::string>;

} // namespace amanuensis::core
