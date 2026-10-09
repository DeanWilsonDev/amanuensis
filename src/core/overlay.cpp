#include "amanuensis/core/overlay.hpp"

#include <utility>
#include <variant>

namespace amanuensis::core {

Value Overlay(const Value& base, const Value& overrides)
{
  const auto* baseObject = std::get_if<OrderedMap<Value>>(&base.data);
  const auto* overrideObject = std::get_if<OrderedMap<Value>>(&overrides.data);
  if (baseObject == nullptr || overrideObject == nullptr) {
    return overrides;
  }

  OrderedMap<Value> merged = *baseObject;
  for (const auto& [key, overrideValue] : overrideObject->GetEntries()) {
    const Value* baseValue = merged.Find(key);
    if (baseValue != nullptr) {
      merged.Insert(key, Overlay(*baseValue, overrideValue));
    }
    else {
      merged.Insert(key, overrideValue);
    }
  }
  return Value{std::move(merged)};
}

} // namespace amanuensis::core
