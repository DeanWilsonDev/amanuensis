#pragma once

#include <amanuensis/json-value.hpp>
#include <amanuensis/json.hpp>

namespace Amanuensis {

// -----------------------------------------------------------------------
// WriteArchive — handed to Serialise functions when converting T → JsonValue
// -----------------------------------------------------------------------

class WriteArchive {
public:
  WriteArchive()
      : object_(Json::MakeObject())
  {
  }

  template <typename FieldType> void Field(const char* jsonKey, const FieldType& fieldJsonValue);

  JsonValue& GetJsonValue() { return object_; }

private:
  JsonValue object_;
};

} // namespace Amanuensis
