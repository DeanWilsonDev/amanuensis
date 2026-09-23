#pragma once

#include "amanuensis/value-traits.hpp"

namespace Amanuensis {

template <
    typename SourceValue,
    typename TargetValue,
    typename TargetTraits = ValueTraits<TargetValue>,
    typename SourceTraits = ValueTraits<SourceValue>>
class Converter {
public:
  Converter() = delete;

  static TargetValue ConvertValue(const SourceValue& source)
  {
    switch (SourceTraits::GetType(source)) {
    case JsonValueType::Null:
      return TargetTraits::MakeNull();
    case JsonValueType::Boolean:
      return TargetTraits::MakeBoolean(SourceTraits::AsBoolean(source));
    case JsonValueType::Integer:
      return TargetTraits::MakeInteger(SourceTraits::AsInteger(source));
    case JsonValueType::Double:
      return TargetTraits::MakeDouble(SourceTraits::AsDouble(source));
    case JsonValueType::String:
      return TargetTraits::MakeString(SourceTraits::AsString(source));
    case JsonValueType::Array: {
      auto target = TargetTraits::MakeArray();
      for (const auto& element : SourceTraits::AsArray(source)) {
        TargetTraits::PushBack(target, ConvertValue(element));
      }
      return target;
    }
    case JsonValueType::Object: {
      auto target = TargetTraits::MakeObject();
      for (const auto& [key, element] : SourceTraits::AsObject(source)) {
        TargetTraits::Insert(target, key, ConvertValue(element));
      }
      return target;
    }
    }

    throw TypeMismatchError("Unsupported source value type");
  }
};

} // namespace Amanuensis
