#include "amanuensis/core/diff.hpp"

#include <bit>
#include <cstdint>
#include <string>
#include <variant>

namespace amanuensis::core {

// Whole-value comparison for everything Diff doesn't recurse into. Objects
// inside arrays ignore key order, and doubles compare by bit pattern, the
// same as StableHash.
static bool Equal(const Value& left, const Value& right)
{
  if (left.data.index() != right.data.index()) {
    return false;
  }
  if (const auto* leftDouble = std::get_if<double>(&left.data)) {
    return std::bit_cast<std::uint64_t>(*leftDouble) ==
           std::bit_cast<std::uint64_t>(std::get<double>(right.data));
  }
  if (const auto* leftArray = std::get_if<std::vector<Value>>(&left.data)) {
    const auto& rightArray = std::get<std::vector<Value>>(right.data);
    if (leftArray->size() != rightArray.size()) {
      return false;
    }
    for (std::size_t index = 0; index < leftArray->size(); ++index) {
      if (!Equal((*leftArray)[index], rightArray[index])) {
        return false;
      }
    }
    return true;
  }
  if (const auto* leftObject = std::get_if<OrderedMap<Value>>(&left.data)) {
    const auto& rightObject = std::get<OrderedMap<Value>>(right.data);
    if (leftObject->GetEntries().size() != rightObject.GetEntries().size()) {
      return false;
    }
    for (const auto& [key, leftValue] : leftObject->GetEntries()) {
      const Value* rightValue = rightObject.Find(key);
      if (rightValue == nullptr || !Equal(leftValue, *rightValue)) {
        return false;
      }
    }
    return true;
  }
  if (const auto* leftBoolean = std::get_if<bool>(&left.data)) {
    return *leftBoolean == std::get<bool>(right.data);
  }
  if (const auto* leftInteger = std::get_if<long long>(&left.data)) {
    return *leftInteger == std::get<long long>(right.data);
  }
  if (const auto* leftString = std::get_if<std::string>(&left.data)) {
    return *leftString == std::get<std::string>(right.data);
  }
  return true; // both Null
}

static void CollectChanges(
    const Value& base,
    const Value& current,
    KeyPath& path,
    std::vector<KeyPath>& changes
)
{
  const auto* baseObject = std::get_if<OrderedMap<Value>>(&base.data);
  const auto* currentObject = std::get_if<OrderedMap<Value>>(&current.data);
  if (baseObject == nullptr || currentObject == nullptr) {
    if (!Equal(base, current)) {
      changes.push_back(path);
    }
    return;
  }

  for (const auto& [key, currentValue] : currentObject->GetEntries()) {
    path.push_back(key);
    const Value* baseValue = baseObject->Find(key);
    if (baseValue == nullptr) {
      changes.push_back(path);
    }
    else {
      CollectChanges(*baseValue, currentValue, path, changes);
    }
    path.pop_back();
  }
  for (const auto& [key, baseValue] : baseObject->GetEntries()) {
    if (!currentObject->Contains(key)) {
      path.push_back(key);
      changes.push_back(path);
      path.pop_back();
    }
  }
}

std::vector<KeyPath> Diff(const Value& base, const Value& current)
{
  std::vector<KeyPath> changes;
  KeyPath path;
  CollectChanges(base, current, path, changes);
  return changes;
}

} // namespace amanuensis::core
