#include "calamus/parser/header/parselets/type-field-parselet.hpp"
#include "calamus/syntax.hpp"

#include <string>
#include <string_view>

namespace amanuensis::calamus {

MaybeError TypeFieldParselet::Parse(LineCursor& cursor, Header& header) const
{
  std::string_view value = cursor.Rest();
  if (!IsTypeName(value)) {
    return cursor.ErrorHere(
        "@type must be a name: a letter or '_', then letters, digits, '_', '.' or ':'"
    );
  }
  header.type = std::string(value);
  return std::nullopt;
}

} // namespace amanuensis::calamus
