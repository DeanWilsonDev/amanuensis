#pragma once

#include "amanuensis/core/parse-result.hpp"
#include "core/cursor.hpp"
#include "json/parser/value-parselet-table.hpp"

namespace amanuensis::json {

class ValueParser {
public:
  core::ParseResult Parse(core::Cursor& cursor) const;

private:
  const ValueParseletTable& parselets = ValueParseletTable::Instance();
};

} // namespace amanuensis::json
