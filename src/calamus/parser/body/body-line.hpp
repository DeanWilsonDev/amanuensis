#pragma once

#include "amanuensis/core/value.hpp"

#include <optional>
#include <string>

namespace amanuensis::calamus {

enum class LineKind {
  Field,     // key : value
  Block,     // key:
  Item,      // - value
  ItemBlock, // -
};

struct BodyLine {
  int number = 0;
  bool marked = false;
  int depth = 0;
  LineKind kind = LineKind::Field;
  std::string key;
  int keyColumn = 0;
  core::Value value;
  std::optional<core::Value> recordedDefault;

  bool IsItem() const { return kind == LineKind::Item || kind == LineKind::ItemBlock; }
  int ContentColumn() const { return 3 + depth * 2; }
};

} // namespace amanuensis::calamus
