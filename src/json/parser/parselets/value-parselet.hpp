#pragma once

#include "amanuensis/core/parse-result.hpp"
#include "core/cursor.hpp"

namespace amanuensis::json {

class ValueParser;

class ValueParselet {
public:
  virtual ~ValueParselet() = default;
  virtual core::ParseResult Parse(core::Cursor& cursor, const ValueParser& values) const = 0;
};

} // namespace amanuensis::json
