#pragma once

#include "calamus/parser/body/body-line.hpp"
#include "calamus/parser/body/line-end-parser.hpp"
#include "calamus/parser/line-cursor.hpp"
#include "calamus/parser/maybe-error.hpp"
#include "calamus/parser/values/value-parser.hpp"

namespace amanuensis::calamus {

class ItemLineParser {
public:
  ItemLineParser(const ValueParser& values, const LineEndParser& lineEnd)
      : values(values)
      , lineEnd(lineEnd)
  {
  }

  MaybeError Parse(LineCursor& cursor, BodyLine& bodyLine) const;

private:
  const ValueParser& values;
  const LineEndParser& lineEnd;
};

} // namespace amanuensis::calamus
