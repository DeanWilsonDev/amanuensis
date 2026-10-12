#pragma once

#include "calamus/parser/body/body-line.hpp"
#include "calamus/parser/line-cursor.hpp"
#include "calamus/parser/maybe-error.hpp"
#include "calamus/parser/values/value-parser.hpp"

namespace amanuensis::calamus {

class LineEndParser {
public:
  explicit LineEndParser(const ValueParser& values)
      : values(values)
  {
  }

  MaybeError Parse(LineCursor& cursor, BodyLine& bodyLine, const char* after) const;

private:
  const ValueParser& values;
};

} // namespace amanuensis::calamus
