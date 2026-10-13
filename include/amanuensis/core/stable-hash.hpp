#pragma once

#include "amanuensis/core/value.hpp"

#include <cstdint>

namespace amanuensis::core {

// FNV-1a 64 over a fixed encoding of the value, so the result is the same on
// every compiler and platform and never depends on how the value was written
// as text. Two values that Diff finds no difference between hash the same.
//
// Each value feeds a type tag byte and then its payload. Every multi-byte
// number is 8 bytes little-endian.
//
//   Null     0
//   Boolean  1, then 0 or 1
//   Integer  2, then the two's-complement bits
//   Double   3, then the IEEE 754 bits, so 0.0 and -0.0 differ
//   String   4, then the byte length and the bytes
//   Array    5, then the element count and each element
//   Object   6, then the entry count and each entry as a String-style key
//            (length and bytes, no tag) followed by its value, with entries
//            sorted bytewise by key so key order never changes the hash
std::uint64_t StableHash(const Value& value);

} // namespace amanuensis::core
