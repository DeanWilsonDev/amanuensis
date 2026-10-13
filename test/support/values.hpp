#pragma once

#include <amanuensis/core/value.hpp>
#include <amanuensis/json/reader.hpp>
#include <amanuensis/json/writer.hpp>

#include <string>
#include <string_view>

namespace core = amanuensis::core;

inline core::Value Json(std::string_view text)
{
  return amanuensis::json::Reader::ParseString(text).value;
}

inline std::string Minified(const core::Value& value)
{
  amanuensis::json::WriterOptions options;
  options.pretty = false;
  options.trailingNewline = false;
  return amanuensis::json::Writer::WriteToString(value, options);
}
