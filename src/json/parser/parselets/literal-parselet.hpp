#pragma once

#include "amanuensis/core/value.hpp"
#include "json/parser/parselets/value-parselet.hpp"

#include <string_view>
#include <utility>

namespace amanuensis::json {

class LiteralParselet : public ValueParselet {
public:
  LiteralParselet(std::string_view text, core::Value value)
      : text(text)
      , value(std::move(value))
  {
  }

  core::ParseResult Parse(core::Cursor& cursor, const ValueParser& values) const override;

private:
  std::string_view text;
  core::Value value;
};

} // namespace amanuensis::json
