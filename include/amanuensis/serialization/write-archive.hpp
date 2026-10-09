#pragma once

#include <amanuensis/core/value.hpp>
#include <amanuensis/json.hpp>

namespace amanuensis {

// -----------------------------------------------------------------------
// WriteArchive — handed to Serialise functions when converting T → Value
// -----------------------------------------------------------------------

class WriteArchive {
public:
  WriteArchive()
      : object_(Json::MakeObject())
  {
  }

  template <typename FieldType> void Field(const char* jsonKey, const FieldType& fieldValue);

  core::Value& GetValue() { return object_; }

private:
  core::Value object_;
};

} // namespace amanuensis
