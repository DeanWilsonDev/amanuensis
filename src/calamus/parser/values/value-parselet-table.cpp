#include "calamus/parser/values/value-parselet-table.hpp"
#include "calamus/parser/messages.hpp"
#include "calamus/parser/values/parselets/inline-array-parselet.hpp"
#include "calamus/parser/values/parselets/number-parselet.hpp"
#include "calamus/parser/values/parselets/quoted-string-parselet.hpp"
#include "calamus/parser/values/parselets/rejected-value-parselet.hpp"
#include "calamus/parser/values/parselets/word-parselet.hpp"
#include "calamus/syntax.hpp"

namespace amanuensis::calamus {

ValueParseletTable::ValueParseletTable()
{
  static const QuotedStringParselet quotedString;
  static const InlineArrayParselet inlineArray;
  static const NumberParselet number;
  static const WordParselet word;
  static const RejectedValueParselet inlineObject{
      "Objects can't be written inline; open a block with 'key:' instead"
  };
  static const RejectedValueParselet comment{CommentError};

  this->Register('"', quotedString);
  this->Register('[', inlineArray);
  this->Register('-', number);
  for (char digit = '0'; digit <= '9'; ++digit) {
    this->Register(digit, number);
  }
  for (std::size_t index = 0; index < CHARACTER_COUNT; ++index) {
    const auto leading = static_cast<char>(index);
    if (IsIdentifierStart(leading)) {
      this->Register(leading, word);
    }
  }
  this->Register('{', inlineObject);
  this->Register('#', comment);
}

} // namespace amanuensis::calamus
