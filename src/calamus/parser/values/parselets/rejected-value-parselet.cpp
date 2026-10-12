#include "calamus/parser/values/parselets/rejected-value-parselet.hpp"

namespace amanuensis::calamus {

core::ParseResult RejectedValueParselet::Parse(LineCursor& cursor, const ValueParser&) const
{
  return cursor.MakeError(this->message);
}

} // namespace amanuensis::calamus
