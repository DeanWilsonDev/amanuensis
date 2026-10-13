#pragma once

#include "calamus/parser/values/parselets/value-parselet.hpp"

#include <array>
#include <cstddef>

namespace amanuensis::calamus {

class ValueParseletTable {
public:
  ValueParseletTable(const ValueParseletTable&) = delete;
  ValueParseletTable& operator=(const ValueParseletTable&) = delete;
  ValueParseletTable(ValueParseletTable&&) = delete;
  ValueParseletTable& operator=(ValueParseletTable&&) = delete;

  static const ValueParseletTable& Instance()
  {
    static const ValueParseletTable instance;
    return instance;
  }

  const ValueParselet* Find(char leading) const
  {
    return this->parselets[static_cast<unsigned char>(leading)];
  }

private:
  ValueParseletTable();

  void Register(char leading, const ValueParselet& parselet)
  {
    this->parselets[static_cast<unsigned char>(leading)] = &parselet;
  }

  static constexpr std::size_t CHARACTER_COUNT = 256;

  std::array<const ValueParselet*, CHARACTER_COUNT> parselets{};
};

} // namespace amanuensis::calamus
