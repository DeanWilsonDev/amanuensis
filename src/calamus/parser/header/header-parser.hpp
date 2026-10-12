#pragma once

#include "amanuensis/calamus/header.hpp"
#include "calamus/parser/header/header-field-table.hpp"
#include "calamus/parser/maybe-error.hpp"
#include "calamus/parser/source-text.hpp"

#include <cstddef>
#include <functional>
#include <set>
#include <string>

namespace amanuensis::calamus {

class HeaderParser {
public:
  MaybeError ParseLine(const SourceLine& line);
  MaybeError CheckRequired(const SourceText& source, std::size_t bodyStart) const;
  Header TakeHeader();

private:
  const HeaderFieldTable& fields = HeaderFieldTable::Instance();
  Header header;
  std::set<std::string, std::less<>> seen;
};

} // namespace amanuensis::calamus
