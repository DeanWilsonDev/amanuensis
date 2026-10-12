#pragma once

#include "amanuensis/core/value.hpp"

#include <string>

namespace amanuensis::calamus {

bool IsInline(const core::Value& value);

// value must be inline. Returns false, partway through, on NaN or infinity.
bool AppendInlineValue(std::string& output, const core::Value& value);

} // namespace amanuensis::calamus
