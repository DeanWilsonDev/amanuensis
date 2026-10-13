#include "json/parser/parselets/string-parselet.hpp"
#include "core/quoted-string/quoted-string.hpp"

namespace amanuensis::json {

core::ParseResult StringParselet::Parse(core::Cursor& cursor, const ValueParser&) const
{
  return core::ParseQuotedString(cursor);
}

} // namespace amanuensis::json
