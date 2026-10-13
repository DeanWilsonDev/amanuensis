#pragma once

#include "amanuensis/core/value.hpp"

#include <string>

namespace amanuensis::calamus {

// Calamus writes each double in its canonical text: the shortest text that
// reads back as the same float when the double is exactly a float, and
// otherwise the shortest text that reads back as the same double. So a float
// 0.1 is written "0.1", not "0.10000000149011612".

// value must be finite.
void AppendCanonicalDouble(std::string& output, double value);

// The double that value's canonical text reads back as. NaN and infinity have
// no text, so they come back unchanged.
double CanonicalDouble(double value);

// The text can't tell a float 0.1 from a double 0.1, so the reader and writer
// compare and hash canonical values; core::Diff and core::StableHash stay
// bit-exact.
core::Value Canonicalise(const core::Value& value);

} // namespace amanuensis::calamus
