#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace amanuensis::calamus {

std::optional<std::uint64_t> ParseHex(std::string_view text, std::size_t maxDigits);

} // namespace amanuensis::calamus
