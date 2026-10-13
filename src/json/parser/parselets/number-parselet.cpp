#include "json/parser/parselets/number-parselet.hpp"
#include "core/number-format.hpp"

namespace amanuensis::json {

core::ParseResult NumberParselet::Parse(core::Cursor& cursor, const ValueParser&) const
{
  return core::ParseNumber(cursor);
}

} // namespace amanuensis::json
