#pragma once

#include "amanuensis/core/parse-result.hpp"

#include <cstddef>
#include <string>
#include <string_view>

namespace amanuensis::core {

class Cursor {
public:
  explicit Cursor(std::string_view input)
      : input_(input)
  {
  }

  bool IsAtEnd() const { return position_ >= input_.size(); }

  char Peek() const { return IsAtEnd() ? '\0' : input_[position_]; }

  // Must not be called at the end.
  char Advance()
  {
    char current = input_[position_];
    ++position_;
    if (current == '\n') {
      ++line_;
      column_ = 1;
    }
    else {
      ++column_;
    }
    return current;
  }

  std::size_t Position() const { return position_; }
  int Line() const { return line_; }
  int Column() const { return column_; }

  std::string_view Slice(std::size_t start) const
  {
    return input_.substr(start, position_ - start);
  }

  ParseResult MakeError(const std::string& message) const
  {
    return ParseResult{false, Value(), ParseError{message, line_, column_}};
  }

private:
  std::string_view input_;
  std::size_t position_ = 0;
  int line_ = 1;
  int column_ = 1;
};

} // namespace amanuensis::core
