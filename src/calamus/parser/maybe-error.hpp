#pragma once

#include "amanuensis/core/parse-error.hpp"

#include <optional>

namespace amanuensis::calamus {

using MaybeError = std::optional<core::ParseError>;

} // namespace amanuensis::calamus
