#include "calamus/parser/header/header-field-table.hpp"
#include "calamus/parser/header/parselets/base-field-parselet.hpp"
#include "calamus/parser/header/parselets/base-hash-field-parselet.hpp"
#include "calamus/parser/header/parselets/id-field-parselet.hpp"
#include "calamus/parser/header/parselets/type-field-parselet.hpp"
#include "calamus/parser/header/parselets/version-field-parselet.hpp"

namespace amanuensis::calamus {

HeaderFieldTable::HeaderFieldTable()
{
  static const VersionFieldParselet version;
  static const TypeFieldParselet type;
  static const IdFieldParselet id;
  static const BaseFieldParselet base;
  static const BaseHashFieldParselet baseHash;

  this->RegisterRequired("version", version);
  this->RegisterRequired("type", type);
  this->Register("id", id);
  this->Register("base", base);
  this->Register("base_hash", baseHash);
}

const HeaderFieldParselet* HeaderFieldTable::Find(std::string_view name) const
{
  for (const Entry& entry : this->entries) {
    if (entry.name == name) {
      return entry.parselet;
    }
  }
  return nullptr;
}

void HeaderFieldTable::Register(std::string_view name, const HeaderFieldParselet& parselet)
{
  this->entries.push_back(Entry{name, &parselet});
}

void HeaderFieldTable::RegisterRequired(std::string_view name, const HeaderFieldParselet& parselet)
{
  this->Register(name, parselet);
  this->requiredNames.push_back(name);
}

} // namespace amanuensis::calamus
