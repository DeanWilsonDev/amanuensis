#pragma once

#include "amanuensis/core/key-path.hpp"
#include "amanuensis/core/value.hpp"

#include <vector>

namespace amanuensis::core {

// Where both hold an Object, Diff recurses key by key, so key order never
// counts as a change. Anywhere else the two values compare whole: an Integer
// never equals a Double, doubles compare by bit pattern, and arrays compare
// element by element. The root differing as a whole is the empty path.
//
// Paths come in document order: current's keys first, then the keys only base
// has.
std::vector<KeyPath> Diff(const Value& base, const Value& current);

} // namespace amanuensis::core
