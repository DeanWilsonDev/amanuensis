#pragma once

#include "amanuensis/calamus/header.hpp"
#include "amanuensis/core/value.hpp"

#include <filesystem>
#include <string>

namespace amanuensis::calamus {

struct WriteResult {
  bool succeeded;
  std::string text;
  std::string error;
};

class Writer {
public:
  // Writes entity, which must be an Object, as Calamus text over its base: the
  // prefab, or the type's default when there's no @base. The writer marks the
  // fields that differ from the base, records the base value each marked
  // field overrides after '|', and sets @base_hash from the base, ignoring
  // header.baseHash. Keys only the base has are left out, since Calamus can't
  // remove a field. The same input always gives the same text.
  //
  // Fails without throwing, naming the field, on NaN or infinity, which have
  // no syntax, and on a header the reader would reject.
  static WriteResult
  WriteToString(const Header& header, const core::Value& entity, const core::Value& base);

  static WriteResult WriteToFile(
      const Header& header,
      const core::Value& entity,
      const core::Value& base,
      const std::filesystem::path& path
  );
};

} // namespace amanuensis::calamus
