#include "calamus/parser/header/parselets/id-field-parselet.hpp"
#include "calamus/parser/header/hex.hpp"

namespace amanuensis::calamus {

MaybeError IdFieldParselet::Parse(LineCursor& cursor, Header& header) const
{
  header.id = ParseHex(cursor.Rest(), 16);
  if (!header.id) {
    return cursor.ErrorHere("@id must be 1 to 16 hex digits");
  }
  return std::nullopt;
}

} // namespace amanuensis::calamus
