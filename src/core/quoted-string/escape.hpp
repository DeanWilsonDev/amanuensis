#pragma once

#include "amanuensis/core/parse-error.hpp"
#include "core/cursor.hpp"

#include <optional>
#include <string>

namespace amanuensis::core {

// Reads the escape after a backslash the caller has consumed.
std::optional<ParseError> ReadEscape(Cursor& cursor, std::string& output);

} // namespace amanuensis::core
