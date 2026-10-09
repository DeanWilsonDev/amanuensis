#include "amanuensis/core/stable-hash.hpp"

#include <algorithm>
#include <bit>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace amanuensis::core {

namespace {

// The type tags are part of the hash's definition, so they're spelled out
// here rather than taken from ValueType.
enum class HashTag : std::uint8_t {
  Null = 0,
  Boolean = 1,
  Integer = 2,
  Double = 3,
  String = 4,
  Array = 5,
  Object = 6,
};

class Fnv1a64 {
public:
  void Byte(std::uint8_t byte)
  {
    hash_ ^= byte;
    hash_ *= kPrime;
  }

  void Tag(HashTag tag) { Byte(static_cast<std::uint8_t>(tag)); }

  void Word(std::uint64_t word)
  {
    for (int shift = 0; shift < 64; shift += 8) {
      Byte(static_cast<std::uint8_t>(word >> shift));
    }
  }

  void Text(std::string_view text)
  {
    Word(static_cast<std::uint64_t>(text.size()));
    for (char character : text) {
      Byte(static_cast<std::uint8_t>(character));
    }
  }

  std::uint64_t Result() const { return hash_; }

private:
  static constexpr std::uint64_t kOffsetBasis = 0xcbf29ce484222325ULL;
  static constexpr std::uint64_t kPrime = 0x100000001b3ULL;

  std::uint64_t hash_ = kOffsetBasis;
};

void Feed(Fnv1a64& hasher, const Value& value)
{
  if (const auto* boolean = std::get_if<bool>(&value.data)) {
    hasher.Tag(HashTag::Boolean);
    hasher.Byte(*boolean ? 1 : 0);
  }
  else if (const auto* integer = std::get_if<long long>(&value.data)) {
    hasher.Tag(HashTag::Integer);
    hasher.Word(static_cast<std::uint64_t>(*integer));
  }
  else if (const auto* number = std::get_if<double>(&value.data)) {
    hasher.Tag(HashTag::Double);
    hasher.Word(std::bit_cast<std::uint64_t>(*number));
  }
  else if (const auto* text = std::get_if<std::string>(&value.data)) {
    hasher.Tag(HashTag::String);
    hasher.Text(*text);
  }
  else if (const auto* array = std::get_if<std::vector<Value>>(&value.data)) {
    hasher.Tag(HashTag::Array);
    hasher.Word(static_cast<std::uint64_t>(array->size()));
    for (const Value& element : *array) {
      Feed(hasher, element);
    }
  }
  else if (const auto* object = std::get_if<OrderedMap<Value>>(&value.data)) {
    const auto& entries = object->GetEntries();
    std::vector<const std::pair<std::string, Value>*> sorted;
    sorted.reserve(entries.size());
    for (const auto& entry : entries) {
      sorted.push_back(&entry);
    }
    std::sort(sorted.begin(), sorted.end(), [](const auto* left, const auto* right) {
      return left->first < right->first;
    });

    hasher.Tag(HashTag::Object);
    hasher.Word(static_cast<std::uint64_t>(sorted.size()));
    for (const auto* entry : sorted) {
      hasher.Text(entry->first);
      Feed(hasher, entry->second);
    }
  }
  else {
    hasher.Tag(HashTag::Null);
  }
}

} // namespace

std::uint64_t StableHash(const Value& value)
{
  Fnv1a64 hasher;
  Feed(hasher, value);
  return hasher.Result();
}

} // namespace amanuensis::core
