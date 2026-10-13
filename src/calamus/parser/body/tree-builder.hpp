#pragma once

#include "amanuensis/calamus/reader.hpp"
#include "amanuensis/core/key-path.hpp"
#include "amanuensis/core/value.hpp"
#include "calamus/parser/body/body-line.hpp"
#include "calamus/parser/maybe-error.hpp"

#include <cstddef>
#include <vector>

namespace amanuensis::calamus {

class TreeBuilder {
public:
  TreeBuilder(std::vector<BodyLine>& lines, std::vector<Field>& fields)
      : lines(lines)
      , fields(fields)
  {
  }

  MaybeError Build(core::Value& root);

private:
  // A Null container takes its kind from its first line: fields make an Object,
  // '-' items an Array.
  MaybeError ReadChildren(int depth, core::Value& container, core::KeyPath& path, bool insideArray);
  MaybeError CheckDepth(const BodyLine& line, int depth) const;
  MaybeError CheckKind(const BodyLine& line, const core::Value& container, int depth) const;
  MaybeError ReadItem(BodyLine& line, core::Value& container, core::KeyPath& path, int depth);
  MaybeError ReadKeyed(
      BodyLine& line,
      core::Value& container,
      core::KeyPath& path,
      int depth,
      bool insideArray
  );
  // A block with no lines under it is an empty Object.
  MaybeError ReadBlock(int depth, core::Value& block, core::KeyPath& path, bool insideArray);

  std::vector<BodyLine>& lines;
  std::vector<Field>& fields;
  std::size_t index = 0;
};

} // namespace amanuensis::calamus
