#pragma once

#include "amanuensis/calamus/header.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace amanuensis::calamus {

std::optional<std::string> ValidateHeader(const Header& header);
std::string WriteHeader(const Header& header, std::uint64_t baseHash);

} // namespace amanuensis::calamus
