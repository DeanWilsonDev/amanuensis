#pragma once

#include <string>

namespace amanuensis::core {

// Numbers are written the same way in every format. Integers are plain
// decimal. Doubles use std::to_chars' shortest form, which reads back
// bit-identical, with ".0" added when that form has neither a '.' nor an
// exponent, so the text reads back as a Double rather than an Integer.

void AppendInteger(std::string& output, long long value);

// value must be finite. Each format decides how to write NaN and infinity.
void AppendDouble(std::string& output, double value);

} // namespace amanuensis::core
