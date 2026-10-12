#pragma once

#include "calamus/parser/values/parselets/value-parselet.hpp"

namespace amanuensis::calamus {

class NumberParselet : public ValueParselet {
public:
  core::ParseResult Parse(LineCursor& cursor, const ValueParser& values) const override;
};

} // namespace amanuensis::calamus
