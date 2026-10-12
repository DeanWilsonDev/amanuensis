#pragma once

#include "calamus/parser/values/parselets/value-parselet.hpp"

namespace amanuensis::calamus {

class RejectedValueParselet : public ValueParselet {
public:
  explicit RejectedValueParselet(const char* message)
      : message(message)
  {
  }

  core::ParseResult Parse(LineCursor& cursor, const ValueParser& values) const override;

private:
  const char* message;
};

} // namespace amanuensis::calamus
