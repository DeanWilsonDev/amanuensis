#pragma once

#include "calamus/parser/body/body-line.hpp"
#include "calamus/parser/line-cursor.hpp"
#include "calamus/parser/maybe-error.hpp"

namespace amanuensis::calamus {

class LinePrefixParser {
public:
  MaybeError Parse(LineCursor& cursor, BodyLine& bodyLine) const;

private:
  MaybeError ParseMarker(LineCursor& cursor, BodyLine& bodyLine) const;
  MaybeError ParseIndentation(LineCursor& cursor, BodyLine& bodyLine) const;
};

} // namespace amanuensis::calamus
