#pragma once

#include "calamus/canonical.hpp"
#include "support/values.hpp"
#include <amanuensis/calamus/reader.hpp>
#include <amanuensis/calamus/writer.hpp>

#include <bit>
#include <cstdint>
#include <string>

namespace calamus = amanuensis::calamus;

inline calamus::Header DoorHeader()
{
  calamus::Header header;
  header.version = 1;
  header.type = "Door";
  return header;
}

inline std::string Write(const core::Value& entity, const core::Value& base)
{
  auto result = calamus::Writer::WriteToString(DoorHeader(), entity, base);
  return result.succeeded ? result.text : "error: " + result.error;
}

// A parse error comes back as the loaded value, so a test that compares the
// value shows it.
inline calamus::LoadResult Load(const std::string& text, const core::Value& base)
{
  auto read = calamus::Reader::ParseString(text);
  if (!read.succeeded) {
    calamus::LoadResult failed;
    failed.value = core::Value{"parse error: " + read.error.message};
    return failed;
  }
  return calamus::Reader::Load(read.document, base);
}

inline std::string Canonical(const core::Value& value)
{
  return Minified(calamus::Canonicalise(value));
}

inline std::uint64_t Bits(double value)
{
  return std::bit_cast<std::uint64_t>(value);
}
