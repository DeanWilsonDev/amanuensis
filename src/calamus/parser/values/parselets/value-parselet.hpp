#pragma once

#include "amanuensis/core/parse-result.hpp"
#include "calamus/parser/line-cursor.hpp"

namespace amanuensis::calamus {

class ValueParser;

class ValueParselet {
public:
  virtual ~ValueParselet() = default;
  virtual core::ParseResult Parse(LineCursor& cursor, const ValueParser& values) const = 0;
};

} // namespace amanuensis::calamus
