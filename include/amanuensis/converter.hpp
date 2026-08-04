#pragma once

#include "amanuensis/value.hpp"
#include "amanuensis/json.hpp"
#include "amanuensis/value-traits.hpp"

namespace Amanuensis {

template <typename TargetValue> class Converter {
public:
  Converter() = delete;

  static TargetValue FromValue(const Value& source)
  {
    if (Json::IsNull(source))
      return ValueTraits<TargetValue>::MakeNull();
    if (Json::IsBoolean(source))
      return ValueTraits<TargetValue>::MakeBoolean(Json::AsBoolean(source));
    if (Json::IsInteger(source))
      return ValueTraits<TargetValue>::MakeInteger(Json::AsInteger(source));
    if (Json::IsDouble(source))
      return ValueTraits<TargetValue>::MakeDouble(Json::AsDouble(source));
    if (Json::IsString(source))
      return ValueTraits<TargetValue>::MakeString(Json::AsString(source));

    if (Json::IsArray(source)) {
      auto target = ValueTraits<TargetValue>::MakeArray();
      for (const auto& element : Json::AsArray(source)) {
        ValueTraits<TargetValue>::PushBack(target, FromValue(element));
      }
      return target;
    }

    if (Json::IsObject(source)) {
      auto target = ValueTraits<TargetValue>::MakeObject();
      for (auto it = Json::BeginObject(source); it != Json::EndObject(source); ++it) {
        ValueTraits<TargetValue>::Insert(target, it->first, FromValue(it->second));
      }
      return target;
    }

    return ValueTraits<TargetValue>::MakeNull();
  }
};

} // namespace Amanuensis
