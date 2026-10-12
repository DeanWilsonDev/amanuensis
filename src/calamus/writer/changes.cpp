#include "calamus/writer/changes.hpp"
#include "amanuensis/core/diff.hpp"
#include "calamus/paths.hpp"

#include <algorithm>
#include <cstddef>
#include <utility>
#include <variant>

namespace amanuensis::calamus {

using Object = core::OrderedMap<core::Value>;

static bool IsUnder(const core::KeyPath& path, const core::KeyPath& prefix)
{
  return path.size() >= prefix.size() && std::equal(prefix.begin(), prefix.end(), path.begin());
}

static bool HasObjectAt(const core::Value& entity, const core::KeyPath& path)
{
  const core::Value* found = FindPath(entity, path);
  return found != nullptr && std::holds_alternative<Object>(found->data);
}

static void
AddDiff(Changes& changes, const core::Value& canonicalBase, const core::Value& canonicalEntity)
{
  for (core::KeyPath& path : core::Diff(canonicalBase, canonicalEntity)) {
    // A key only the base has can't be written, so nothing marks it.
    if (FindPath(canonicalEntity, path) == nullptr) {
      continue;
    }
    for (std::size_t length = 0; length < path.size(); ++length) {
      changes.containing.insert(core::KeyPath(path.begin(), path.begin() + length));
    }
    changes.whole.insert(std::move(path));
  }
}

static void AddAtomicObjects(
    Changes& changes,
    const core::Value& canonicalEntity,
    const std::vector<core::KeyPath>& atomic
)
{
  for (const core::KeyPath& atomicPath : atomic) {
    // Paths with the same prefix sort together, so the first one at or after
    // the atomic path is under it if any is.
    auto firstUnder = changes.whole.lower_bound(atomicPath);
    if (HasObjectAt(canonicalEntity, atomicPath) && firstUnder != changes.whole.end() &&
        IsUnder(*firstUnder, atomicPath)) {
      changes.whole.insert(atomicPath);
    }
  }
}

Changes FindChanges(
    const core::Value& canonicalBase,
    const core::Value& canonicalEntity,
    const std::vector<core::KeyPath>& atomic
)
{
  Changes changes;
  AddDiff(changes, canonicalBase, canonicalEntity);
  AddAtomicObjects(changes, canonicalEntity, atomic);
  return changes;
}

} // namespace amanuensis::calamus
