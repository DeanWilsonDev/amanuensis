#pragma once

#include "amanuensis/core/parse-result.hpp"
#include "core/cursor.hpp"

#include <string>
#include <string_view>

namespace amanuensis::core {

// Reads a quoted string starting at the cursor's opening quote. On success the
// result is a String value and the cursor is past the closing quote.
ParseResult ParseQuotedString(Cursor& cursor);

void AppendQuotedString(std::string& output, std::string_view text);

} // namespace amanuensis::core
