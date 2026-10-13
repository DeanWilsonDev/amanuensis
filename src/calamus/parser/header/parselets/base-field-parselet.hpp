#pragma once

#include "calamus/parser/header/parselets/header-field-parselet.hpp"

namespace amanuensis::calamus {

class BaseFieldParselet : public HeaderFieldParselet {
public:
  MaybeError Parse(LineCursor& cursor, Header& header) const override;
};

} // namespace amanuensis::calamus
