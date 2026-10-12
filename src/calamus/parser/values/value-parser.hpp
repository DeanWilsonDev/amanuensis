#pragma once

#include "amanuensis/core/parse-result.hpp"
#include "calamus/parser/line-cursor.hpp"
#include "calamus/parser/values/value-parselet-table.hpp"

namespace amanuensis::calamus {

class ValueParser {
public:
  core::ParseResult Parse(LineCursor& cursor) const;

private:
  const ValueParseletTable& parselets = ValueParseletTable::Instance();
};

} // namespace amanuensis::calamus
