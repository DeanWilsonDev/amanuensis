#pragma once

#include "support/calamus.hpp"
#include <amanuensis/core/stable-hash.hpp>

#include <cstddef>
#include <cstdio>
#include <string>

namespace calamus_writer_test {

inline std::string Hash(const core::Value& base)
{
  char buffer[17];
  std::snprintf(
      buffer, sizeof(buffer), "%016llx",
      static_cast<unsigned long long>(core::StableHash(calamus::Canonicalise(base)))
  );
  return buffer;
}

inline std::string BodyOf(const core::Value& entity, const core::Value& base)
{
  std::string text = Write(entity, base);
  std::size_t bodyStart = text.find("\n\n");
  return bodyStart == std::string::npos ? text : text.substr(bodyStart + 2);
}

inline calamus::LoadResult RoundTrip(const core::Value& entity, const core::Value& base)
{
  auto written = calamus::Writer::WriteToString(DoorHeader(), entity, base);
  auto read = calamus::Reader::ParseString(written.text);
  if (!written.succeeded || !read.succeeded) {
    calamus::LoadResult failed;
    failed.value = core::Value{"failed: " + written.error + read.error.message};
    return failed;
  }
  return calamus::Reader::Load(read.document, base);
}

inline const core::Value Prefab = Json(R"({
  "position": {"x": 0.0, "y": 0.0, "z": 0.0},
  "scale": {"x": 1.0, "y": 1.0, "z": 1.0},
  "name": "",
  "flags": 160,
  "locked": false,
  "tags": ["puzzle", "act1"],
  "waypoints": [{"x": 0.0, "z": 2.5}],
  "light": {"intensity": 1.0, "colour": {"r": 1.0, "g": 0.9, "b": 0.8}}
})");

inline const core::Value Door = Json(R"({
  "position": {"x": 46.5, "y": 108.25, "z": 0.0},
  "scale": {"x": 1.0, "y": 1.0, "z": 1.0},
  "name": "",
  "flags": 160,
  "locked": true,
  "tags": ["puzzle", "act1"],
  "waypoints": [{"x": 0.0, "z": 2.5}, {"x": 4.0, "z": 2.5}],
  "light": {"intensity": 2.5, "colour": {"r": 1.0, "g": 0.9, "b": 0.8}}
})");

} // namespace calamus_writer_test
