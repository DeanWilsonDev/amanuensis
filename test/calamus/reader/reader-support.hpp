#pragma once

#include "support/values.hpp"
#include <amanuensis/calamus/reader.hpp>

#include <cstddef>
#include <string>
#include <string_view>

namespace calamus = amanuensis::calamus;

namespace calamus_reader_test {

inline const std::string Header = "@version 1\n@type Door\n\n";

inline std::string BodyOf(std::string_view body)
{
  auto result = calamus::Reader::ParseString(Header + std::string(body));
  if (!result.succeeded) {
    return "error: " + result.error.message;
  }
  return Minified(result.document.body);
}

inline std::string ErrorOf(std::string_view text)
{
  auto result = calamus::Reader::ParseString(text);
  if (result.succeeded) {
    return "ok";
  }
  return std::to_string(result.error.line) + ":" + std::to_string(result.error.column) + " " +
         result.error.message;
}

inline std::string BodyErrorOf(std::string_view body)
{
  return ErrorOf(Header + std::string(body));
}

inline std::string FieldsOf(std::string_view body)
{
  auto result = calamus::Reader::ParseString(Header + std::string(body));
  if (!result.succeeded) {
    return "error: " + result.error.message;
  }
  std::string fields;
  for (const calamus::Field& field : result.document.fields) {
    if (!fields.empty()) {
      fields += " ";
    }
    for (std::size_t index = 0; index < field.path.size(); ++index) {
      fields += (index == 0 ? "" : ".") + field.path[index];
    }
    if (field.marked) {
      fields += "*";
    }
  }
  return fields;
}

inline constexpr const char* SpecExample = R"calamus(@version 3
@type Door
@id 3f9c2a7be1d04c58
@base "prefabs/wooden_door"
@base_hash 9f2c41e07b3d5a18

* position:
*   x : 46.103531 | 0.0
*   y : 108.07968 | 0.0
    z : 0.0
  scale:
    x : 1.0
    y : 1.0
    z : 1.0
* orientation:
*   x : 0.0 | 0.0
*   y : 0.0 | 0.0
*   z : -0.069125 | 0.0
*   w : 0.997608 | 1.0
  name : ""
  flags : 160
* locked : true | false
  tags : ["puzzle", "act1"]
  waypoints:
    -
      x : 0.0
      z : 2.5
    -
      x : 4.0
      z : 2.5
* light:
*   intensity : 2.5 | 1.0
    colour:
      r : 1.0
      g : 0.9
      b : 0.8
)calamus";

} // namespace calamus_reader_test
