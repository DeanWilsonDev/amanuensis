#pragma once

#include "support/values.hpp"

#include <bit>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <random>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace conformance_test {

using Array = std::vector<core::Value>;
using Object = core::OrderedMap<core::Value>;

// std::mt19937_64's output is fixed by the standard; the distributions aren't,
// so they're done by hand.
class RandomValues {
public:
  explicit RandomValues(std::uint64_t seed)
      : engine_(seed)
  {
  }

  std::uint64_t Below(std::uint64_t limit) { return engine_() % limit; }

  core::Value Scalar()
  {
    switch (Below(9)) {
    case 0:
      return core::Value();
    case 1:
      return core::Value{Below(2) == 0};
    case 2:
      return core::Value{static_cast<long long>(Below(2001)) - 1000};
    case 3:
      return core::Value{static_cast<long long>(engine_())};
    case 4:
      return core::Value{static_cast<double>(static_cast<long long>(Below(4001)) - 2000) / 8.0};
    case 5: {
      double value = std::bit_cast<double>(engine_());
      return core::Value{std::isfinite(value) ? value : 0.5};
    }
    case 6: {
      float value = std::bit_cast<float>(static_cast<std::uint32_t>(engine_()));
      return core::Value{std::isfinite(value) ? static_cast<double>(value) : 0.25};
    }
    default:
      return core::Value{String()};
    }
  }

  std::string String()
  {
    static const char* pieces[] = {
        "a", "Z", " ", ",", ":", "|", "#", "\"",  "\\", "\n", "\t", "\xC3\xA9", "\xF0\x9F\x98\x80",
        "[", "]", "-", "*", "@", "{", "/", "\x01"
    };
    std::string text;
    for (std::uint64_t count = Below(6); count > 0; --count) {
      text += pieces[Below(std::size(pieces))];
    }
    return text;
  }

  std::string Key()
  {
    static const char* keys[] = {"x",  "y",   "name",     "door frame", "a:b", "",
                                 "#k", "|",   "\xC3\xA9", "_k1",        "9",   "-",
                                 "*",  "@id", "a.b",      "light",      "tags"};
    return keys[Below(std::size(keys))];
  }

  core::Value Any(int depth)
  {
    std::uint64_t kind = depth <= 0 ? 0 : Below(4);
    if (kind == 2) {
      return ArrayOf(depth - 1);
    }
    if (kind == 3) {
      return ObjectOf(depth - 1);
    }
    return Scalar();
  }

  core::Value ArrayOf(int depth)
  {
    Array elements;
    for (std::uint64_t count = Below(4); count > 0; --count) {
      elements.push_back(Any(depth));
    }
    return core::Value{std::move(elements)};
  }

  core::Value ObjectOf(int depth)
  {
    Object object;
    for (std::uint64_t count = Below(5); count > 0; --count) {
      object.Insert(Key(), Any(depth));
    }
    return core::Value{std::move(object)};
  }

  core::Value Mutate(const core::Value& base, int depth)
  {
    const auto* object = std::get_if<Object>(&base.data);
    if (object == nullptr) {
      return Below(3) == 0 ? Any(depth) : base;
    }
    Object mutated;
    for (const auto& [key, value] : object->GetEntries()) {
      std::uint64_t choice = Below(8);
      if (choice == 0) {
        continue;
      }
      if (choice == 1) {
        mutated.Insert(key, Any(depth));
      }
      else if (choice <= 3) {
        mutated.Insert(key, Mutate(value, depth - 1));
      }
      else {
        mutated.Insert(key, value);
      }
    }
    if (Below(3) == 0) {
      mutated.Insert(Key(), Any(depth));
    }
    return core::Value{std::move(mutated)};
  }

private:
  std::mt19937_64 engine_;
};

} // namespace conformance_test
