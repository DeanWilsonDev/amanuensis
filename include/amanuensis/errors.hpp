#pragma once
#include <stdexcept>

namespace Amanuensis {

class TypeMismatchError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

class KeyNotFoundError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

class IndexOutOfRangeError : public std::out_of_range {
public:
  using std::out_of_range::out_of_range;
};

} // namespace Amanuensis
