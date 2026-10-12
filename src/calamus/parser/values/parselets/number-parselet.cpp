#include "calamus/parser/values/parselets/number-parselet.hpp"
#include "core/number-format.hpp"

namespace amanuensis::calamus {

core::ParseResult NumberParselet::Parse(LineCursor& cursor, const ValueParser&) const
{
  return core::ParseNumber(cursor);
}

} // namespace amanuensis::calamus
