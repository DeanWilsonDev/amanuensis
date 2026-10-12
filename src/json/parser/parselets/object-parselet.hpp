#pragma once

#include "json/parser/parselets/value-parselet.hpp"

namespace amanuensis::json {

class ObjectParselet : public ValueParselet {
public:
  core::ParseResult Parse(core::Cursor& cursor, const ValueParser& values) const override;
};

} // namespace amanuensis::json
