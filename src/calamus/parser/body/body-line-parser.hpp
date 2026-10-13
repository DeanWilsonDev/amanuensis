#pragma once

#include "calamus/parser/body/body-line.hpp"
#include "calamus/parser/body/item-line-parser.hpp"
#include "calamus/parser/body/key-line-parser.hpp"
#include "calamus/parser/body/line-end-parser.hpp"
#include "calamus/parser/body/line-prefix-parser.hpp"
#include "calamus/parser/maybe-error.hpp"
#include "calamus/parser/source-text.hpp"
#include "calamus/parser/values/value-parser.hpp"

namespace amanuensis::calamus {

class BodyLineParser {
public:
  BodyLineParser() = default;
  BodyLineParser(const BodyLineParser&) = delete;
  BodyLineParser& operator=(const BodyLineParser&) = delete;

  MaybeError Parse(const SourceLine& line, BodyLine& bodyLine) const;

private:
  ValueParser values;
  LinePrefixParser prefix;
  LineEndParser lineEnd{this->values};
  ItemLineParser item{this->values, this->lineEnd};
  KeyLineParser key{this->values, this->lineEnd};
};

} // namespace amanuensis::calamus
