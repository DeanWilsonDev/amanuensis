#include "calamus/parser/header/parselets/base-hash-field-parselet.hpp"
#include "calamus/parser/header/hex.hpp"

#include <string_view>

namespace amanuensis::calamus {

MaybeError BaseHashFieldParselet::Parse(LineCursor& cursor, Header& header) const
{
  std::string_view value = cursor.Rest();
  header.baseHash = value.size() == 16 ? ParseHex(value, 16) : std::nullopt;
  if (!header.baseHash) {
    return cursor.ErrorHere("@base_hash must be exactly 16 hex digits");
  }
  return std::nullopt;
}

} // namespace amanuensis::calamus
