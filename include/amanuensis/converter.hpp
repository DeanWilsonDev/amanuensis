#pragma once

#include "amanuensis/json.hpp"

namespace Amanuensis {

template <typename SourceValue, typename TargetValue, typename Traits> class Converter {
public:
  Converter() = delete;

  static TargetValue ConvertValue(const SourceValue& source)
  {
    if (Json::IsNull(source))
      return Traits::MakeNull();
    if (Json::IsBoolean(source))
      return Traits::MakeBoolean(Json::AsBoolean(source));
    if (Json::IsInteger(source))
      return Traits::MakeInteger(Json::AsInteger(source));
    if (Json::IsDouble(source))
      return Traits::MakeDouble(Json::AsDouble(source));
    if (Json::IsString(source))
      return Traits::MakeString(Json::AsString(source));

    if (Json::IsArray(source)) {
      auto target = Traits::MakeArray();
      for (const auto& element : Json::AsArray(source)) {
        Traits::PushBack(target, FromValue(element));
      }
      return target;
    }

    if (Json::IsObject(source)) {
      auto target = Traits::MakeObject();
      for (auto it = Json::BeginObject(source); it != Json::EndObject(source); ++it) {
        Traits::Insert(target, it->first, FromValue(it->second));
      }
      return target;
    }

    return Traits::MakeNull();
  }
};

} // namespace Amanuensis
