#pragma once

#include "amanuensis/core/parse-result.hpp"
#include "core/cursor.hpp"

#include <string>

namespace amanuensis::core {

// Reading uses JSON's grammar. A '.' or an exponent makes a Double, anything
// else an Integer, and an integer that overflows long long reads as a Double.
//
// Writing: integers are plain decimal. Doubles use std::to_chars' shortest
// form, which reads back bit-identical, with ".0" added when that form has
// neither a '.' nor an exponent, so the text reads back as a Double rather
// than an Integer.

// Reads a number starting at the cursor, which must be at a '-' or a digit.
// On success the cursor is past the number.
ParseResult ParseNumber(Cursor& cursor);

void AppendInteger(std::string& output, long long value);

// value must be finite. Each format decides how to write NaN and infinity.
void AppendDouble(std::string& output, double value);

// The shortest form that reads back as the same float, with the same ".0"
// rule. value must be finite.
void AppendFloat(std::string& output, float value);

} // namespace amanuensis::core
