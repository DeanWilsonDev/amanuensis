#include "calamus/parser/header/parselets/version-field-parselet.hpp"

#include <charconv>
#include <string_view>
#include <system_error>

namespace amanuensis::calamus {

MaybeError VersionFieldParselet::Parse(LineCursor& cursor, Header& header) const
{
  std::string_view value = cursor.Rest();
  int version = 0;
  auto [endPointer, errorCode] =
      std::from_chars(value.data(), value.data() + value.size(), version);
  if (value.front() < '1' || value.front() > '9' || errorCode != std::errc() ||
      endPointer != value.data() + value.size()) {
    return cursor.ErrorHere("@version must be a whole number of at least 1");
  }
  header.version = version;
  return std::nullopt;
}

} // namespace amanuensis::calamus
