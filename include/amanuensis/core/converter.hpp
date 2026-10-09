#pragma once

#include "amanuensis/core/value-traits.hpp"

namespace amanuensis::core {

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
    case ValueType::Null:
      return TargetTraits::MakeNull();
    case ValueType::Boolean:
      return TargetTraits::MakeBoolean(SourceTraits::AsBoolean(source));
    case ValueType::Integer:
      return TargetTraits::MakeInteger(SourceTraits::AsInteger(source));
    case ValueType::Double:
      return TargetTraits::MakeDouble(SourceTraits::AsDouble(source));
    case ValueType::String:
      return TargetTraits::MakeString(SourceTraits::AsString(source));
    case ValueType::Array: {
      auto target = TargetTraits::MakeArray();
      for (const auto& element : SourceTraits::AsArray(source)) {
        TargetTraits::PushBack(target, ConvertValue(element));
      }
      return target;
    }
    case ValueType::Object: {
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

} // namespace amanuensis::core
