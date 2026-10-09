#pragma once

#include "amanuensis/core/parse-result.hpp"

#include <cstddef>
#include <string>
#include <string_view>

namespace amanuensis::core {

// Walks a text buffer one character at a time and tracks the 1-based line and
// column, so every format reports parse errors the same way.
class Cursor {
public:
  explicit Cursor(std::string_view input)
      : input_(input)
  {
  }

  bool IsAtEnd() const { return position_ >= input_.size(); }

  // The current character, or '\0' at the end of the input.
  char Peek() const { return IsAtEnd() ? '\0' : input_[position_]; }

  // Consumes the current character. Must not be called at the end.
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

  // The input from start up to, but not including, the current position.
  std::string_view Slice(std::size_t start) const
  {
    return input_.substr(start, position_ - start);
  }

  // A failed ParseResult whose error points at the current position.
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
