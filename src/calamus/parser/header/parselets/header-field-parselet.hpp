#pragma once

#include "amanuensis/calamus/header.hpp"
#include "calamus/parser/line-cursor.hpp"
#include "calamus/parser/maybe-error.hpp"

namespace amanuensis::calamus {

class HeaderFieldParselet {
public:
  virtual ~HeaderFieldParselet() = default;
  virtual MaybeError Parse(LineCursor& cursor, Header& header) const = 0;
};

} // namespace amanuensis::calamus
