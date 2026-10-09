#pragma once

#include "amanuensis/core/value.hpp"

namespace amanuensis::core {

// Returns base with overrides applied. Where both hold an Object, the result
// merges them key by key, recursively: it keeps base's key order and appends
// the keys only overrides has. Anywhere else, arrays included, the override
// replaces the base value whole.
Value Overlay(const Value& base, const Value& overrides);

} // namespace amanuensis::core
