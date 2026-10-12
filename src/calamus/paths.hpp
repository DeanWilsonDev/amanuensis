#pragma once

#include "amanuensis/core/key-path.hpp"
#include "amanuensis/core/value.hpp"

#include <string>
#include <utility>
#include <variant>

namespace amanuensis::calamus {

// The value at path, or nullptr when some key on the way is missing or isn't
// an Object.
inline const core::Value* FindPath(const core::Value& root, const core::KeyPath& path)
{
  const core::Value* current = &root;
  for (const std::string& key : path) {
    const auto* object = std::get_if<core::OrderedMap<core::Value>>(&current->data);
    if (object == nullptr || (current = object->Find(key)) == nullptr) {
      return nullptr;
    }
  }
  return current;
}

// Sets the value at path, creating the Objects on the way. No field's path
// runs through another field, so everything on the way is an Object.
inline void SetPath(core::Value& root, const core::KeyPath& path, core::Value value)
{
  core::Value* current = &root;
  for (std::size_t index = 0; index + 1 < path.size(); ++index) {
    auto& object = std::get<core::OrderedMap<core::Value>>(current->data);
    if (!object.Contains(path[index])) {
      object.Insert(path[index], core::Value{core::OrderedMap<core::Value>{}});
    }
    current = &object.Get(path[index]);
  }
  std::get<core::OrderedMap<core::Value>>(current->data).Insert(path.back(), std::move(value));
}

} // namespace amanuensis::calamus
