#include "amanuensis/calamus/migrate.hpp"
#include "calamus/paths.hpp"

#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace amanuensis::calamus {

using Object = core::OrderedMap<core::Value>;

// One marked field for every value in overrides that isn't a non-empty
// Object: the same fields the reader would find in a file that wrote them.
static void AppendFields(const Object& overrides, core::KeyPath& path, std::vector<Field>& fields)
{
  for (const auto& [key, value] : overrides.GetEntries()) {
    path.push_back(key);
    const auto* object = std::get_if<Object>(&value.data);
    if (object != nullptr && object->Size() > 0) {
      AppendFields(*object, path, fields);
    }
    else {
      fields.push_back(Field{path, value, true, std::nullopt, 0});
    }
    path.pop_back();
  }
}

core::MigrateResult Migrate(Document& document, const Migration& migration)
{
  if (document.header.version == migration.currentVersion) {
    return core::MigrateResult{true, std::string()};
  }

  core::Value overrides{Object{}};
  for (const Field& field : document.fields) {
    if (field.marked) {
      SetPath(overrides, field.path, field.value);
    }
  }

  std::string context = "@type " + document.header.type + ": ";
  core::MigrateResult result =
      core::Migrate(overrides, document.header.version, migration.currentVersion, migration.step);
  if (!result.succeeded) {
    result.error = context + result.error;
    return result;
  }
  const auto* overrideObject = std::get_if<Object>(&overrides.data);
  if (overrideObject == nullptr) {
    return core::MigrateResult{false, context + "Migration must leave the overrides an Object"};
  }

  std::vector<Field> fields;
  core::KeyPath path;
  AppendFields(*overrideObject, path, fields);
  document.fields = std::move(fields);
  document.body = std::move(overrides);
  document.header.version = migration.currentVersion;
  return result;
}

} // namespace amanuensis::calamus
