#pragma once

#include "amanuensis/value.hpp"

namespace Amanuensis {

// -----------------------------------------------------------------------
// WriteArchive — handed to Serialise functions when converting T → Value
// -----------------------------------------------------------------------

class WriteArchive {
public:
  WriteArchive()
      : object_(Value::MakeObject())
  {
  }

  template <typename FieldType> void Field(const char* jsonKey, const FieldType& fieldValue);

  Value& GetValue() { return object_; }

private:
  Value object_;
};

} // namespace Amanuensis
