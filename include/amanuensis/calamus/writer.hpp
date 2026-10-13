#pragma once

#include "amanuensis/calamus/header.hpp"
#include "amanuensis/calamus/writer-options.hpp"
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
  // base is the prefab, or the type's default when there's no @base. @base_hash
  // comes from base, and header.baseHash is ignored. Keys only the base has are
  // left out, since Calamus can't remove a field.
  //
  // Fails without throwing, naming the field, on NaN or infinity, which have
  // no syntax, and on a header the reader would reject.
  static WriteResult WriteToString(
      const Header& header,
      const core::Value& entity,
      const core::Value& base,
      const WriterOptions& options = {}
  );

  static WriteResult WriteToFile(
      const Header& header,
      const core::Value& entity,
      const core::Value& base,
      const std::filesystem::path& path,
      const WriterOptions& options = {}
  );
};

} // namespace amanuensis::calamus
