#include "calamus/parser/values/parselets/quoted-string-parselet.hpp"
#include "core/quoted-string.hpp"

namespace amanuensis::calamus {

core::ParseResult QuotedStringParselet::Parse(LineCursor& cursor, const ValueParser&) const
{
  return core::ParseQuotedString(cursor);
}

} // namespace amanuensis::calamus
