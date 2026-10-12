#pragma once

#include "calamus/parser/header/parselets/header-field-parselet.hpp"

#include <string_view>
#include <vector>

namespace amanuensis::calamus {

class HeaderFieldTable {
public:
  HeaderFieldTable(const HeaderFieldTable&) = delete;
  HeaderFieldTable& operator=(const HeaderFieldTable&) = delete;
  HeaderFieldTable(HeaderFieldTable&&) = delete;
  HeaderFieldTable& operator=(HeaderFieldTable&&) = delete;

  static const HeaderFieldTable& Instance()
  {
    static const HeaderFieldTable instance;
    return instance;
  }

  const HeaderFieldParselet* Find(std::string_view name) const;
  const std::vector<std::string_view>& RequiredNames() const { return this->requiredNames; }

private:
  struct Entry {
    std::string_view name;
    const HeaderFieldParselet* parselet;
  };

  HeaderFieldTable();

  void Register(std::string_view name, const HeaderFieldParselet& parselet);
  void RegisterRequired(std::string_view name, const HeaderFieldParselet& parselet);

  std::vector<Entry> entries;
  std::vector<std::string_view> requiredNames;
};

} // namespace amanuensis::calamus
