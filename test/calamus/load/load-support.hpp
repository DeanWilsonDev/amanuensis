#pragma once

#include "support/calamus.hpp"
#include <amanuensis/core/stable-hash.hpp>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>
#include <variant>
#include <vector>

namespace calamus_load_test {

inline std::string HashLine(std::uint64_t hash)
{
  char buffer[40];
  std::snprintf(
      buffer, sizeof(buffer), "@base_hash %016llx\n", static_cast<unsigned long long>(hash)
  );
  return buffer;
}

inline std::string Unchanged(const core::Value& base)
{
  return "@version 1\n@type Door\n" + HashLine(core::StableHash(calamus::Canonicalise(base))) +
         "\n";
}

inline const std::string Changed = "@version 1\n@type Door\n@base_hash 0123456789abcdef\n\n";

inline const std::string Unhashed = "@version 1\n@type Door\n\n";

inline std::string Paths(const std::vector<core::KeyPath>& paths)
{
  std::string joined;
  for (const core::KeyPath& path : paths) {
    if (!joined.empty()) {
      joined += " ";
    }
    for (std::size_t index = 0; index < path.size(); ++index) {
      joined += (index == 0 ? "" : ".") + path[index];
    }
  }
  return joined;
}

inline std::string CanonicalText(double value)
{
  std::string output;
  calamus::AppendCanonicalDouble(output, value);
  return output;
}

inline core::Value FloatBase()
{
  core::Value base = Json(R"({"x":0.0})");
  std::get<core::OrderedMap<core::Value>>(base.data).Insert(
      "x", core::Value{static_cast<double>(0.1f)}
  );
  return base;
}

inline const core::Value Door =
    Json(R"({"position":{"x":0.0,"y":0.0,"z":0.0},"name":"door","locked":false,"tags":["a"]})");

} // namespace calamus_load_test
