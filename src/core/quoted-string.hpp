#pragma once

#include "amanuensis/core/parse-result.hpp"
#include "core/cursor.hpp"

#include <string>
#include <string_view>

namespace amanuensis::core {

// Quoted strings follow JSON's rules in every format: the escapes \" \\ \/ \b
// \f \n \r \t and \uXXXX (with surrogate pairs), and no raw control characters.

// Reads a quoted string starting at the cursor's opening quote. On success the
// result is a String value and the cursor is past the closing quote.
ParseResult ParseQuotedString(Cursor& cursor);

// Appends text as a quoted string that ParseQuotedString reads back unchanged.
void AppendQuotedString(std::string& output, std::string_view text);

} // namespace amanuensis::core
