#pragma once

#include "support/calamus.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace conformance_test {

using Array = std::vector<core::Value>;
using Object = core::OrderedMap<core::Value>;

inline std::string Heal(const std::string& text, const core::Value& base)
{
  return Write(Load(text, base).value, base);
}

inline std::string Replace(std::string text, std::string_view from, std::string_view to)
{
  std::size_t at = text.find(from);
  if (at != std::string::npos) {
    text.replace(at, from.size(), to);
  }
  return text;
}

inline std::string StripMarks(std::string text)
{
  bool lineStart = true;
  for (char& character : text) {
    if (lineStart && character == '*') {
      character = ' ';
    }
    lineStart = character == '\n';
  }
  return text;
}

// Whether every block line with lines under it is marked exactly when one of
// those lines is.
inline bool BlockMarksMatch(std::string_view text)
{
  struct Line {
    bool marked;
    std::size_t depth;
    bool opensBlock;
  };
  std::vector<Line> lines;
  std::size_t start = text.find("\n\n");
  while (start != std::string_view::npos && start + 1 < text.size()) {
    std::size_t end = text.find('\n', start + 1);
    std::string_view line = text.substr(start + 1, end - start - 1);
    start = end;
    if (line.size() < 3) {
      continue;
    }
    std::size_t indent = line.find_first_not_of(' ', 2) - 2;
    lines.push_back(Line{line[0] == '*', indent / 2, line.back() == ':' || line.back() == '-'});
  }
  for (std::size_t index = 0; index < lines.size(); ++index) {
    bool anyChild = false;
    bool anyMarkedChild = false;
    for (std::size_t child = index + 1;
         child < lines.size() && lines[child].depth > lines[index].depth; ++child) {
      anyChild = true;
      anyMarkedChild = anyMarkedChild || lines[child].marked;
    }
    if (lines[index].opensBlock && anyChild && lines[index].marked != anyMarkedChild) {
      return false;
    }
  }
  return true;
}

inline const core::Value& Get(const core::Value& object, const std::string& key)
{
  return std::get<Object>(object.data).Get(key);
}

inline const core::Value Prefab = Json(R"({
  "position": {"x": 0.0, "y": 0.0, "z": 0.0},
  "name": "door",
  "locked": false,
  "tags": ["puzzle"],
  "light": {"intensity": 1.0}
})");

inline const core::Value Door = Json(R"({
  "position": {"x": 46.5, "y": 0.0, "z": 0.0},
  "name": "door",
  "locked": true,
  "tags": ["puzzle"],
  "light": {"intensity": 1.0}
})");

inline const core::Value EditedPrefab = Json(R"({
  "position": {"x": 10.0, "y": 0.0, "z": 0.0},
  "name": "wooden door",
  "locked": false,
  "tags": ["puzzle"],
  "light": {"intensity": 1.0}
})");

} // namespace conformance_test
