#pragma once

#include "amanuensis/core/key-path.hpp"
#include "amanuensis/core/value.hpp"
#include "calamus/writer/changes.hpp"

#include <string>
#include <vector>

namespace amanuensis::calamus {

// Marks come from Changes outside arrays; inside an array every line takes the
// array's mark.
class BodyWriter {
public:
  BodyWriter(std::string& output, const Changes& changes)
      : output(output)
      , changes(changes)
  {
  }

  const std::string& Error() const { return this->error; }

  // base is nullptr when the base has no value at this object's path, or the
  // object is inside an array.
  bool WriteObject(
      const core::OrderedMap<core::Value>& object,
      const core::Value* base,
      bool insideArray,
      bool markAll,
      int depth
  );

private:
  bool WriteEntry(
      const std::string& key,
      const core::Value& value,
      const core::Value* baseValue,
      bool insideArray,
      bool markAll,
      int depth
  );
  bool WriteItems(const std::vector<core::Value>& array, bool marked, int depth);
  void StartLine(bool marked, int depth);
  bool AppendValue(const core::Value& value, const char* where);

  std::string& output;
  const Changes& changes;
  core::KeyPath path;
  std::string error;
};

} // namespace amanuensis::calamus
