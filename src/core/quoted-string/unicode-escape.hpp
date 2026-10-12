#pragma once

#include "amanuensis/core/parse-error.hpp"
#include "core/cursor.hpp"

#include <optional>
#include <string>

namespace amanuensis::core {

// Reads the XXXX of a \uXXXX escape, and a following low surrogate escape when
// it's a high surrogate.
std::optional<ParseError> ReadUnicodeEscape(Cursor& cursor, std::string& output);

} // namespace amanuensis::core
