#include "json/parser/value-parselet-table.hpp"
#include "json/parser/parselets/array-parselet.hpp"
#include "json/parser/parselets/literal-parselet.hpp"
#include "json/parser/parselets/number-parselet.hpp"
#include "json/parser/parselets/object-parselet.hpp"
#include "json/parser/parselets/string-parselet.hpp"

namespace amanuensis::json {

ValueParseletTable::ValueParseletTable()
{
  static const LiteralParselet null{"null", core::Value()};
  static const LiteralParselet trueLiteral{"true", core::Value(true)};
  static const LiteralParselet falseLiteral{"false", core::Value(false)};
  static const StringParselet string;
  static const NumberParselet number;
  static const ArrayParselet array;
  static const ObjectParselet object;

  this->Register('n', null);
  this->Register('t', trueLiteral);
  this->Register('f', falseLiteral);
  this->Register('"', string);
  this->Register('[', array);
  this->Register('{', object);
  this->Register('-', number);
  for (char digit = '0'; digit <= '9'; ++digit) {
    this->Register(digit, number);
  }
}

} // namespace amanuensis::json
