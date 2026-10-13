#pragma once

#include "amanuensis/core/key-path.hpp"

#include <string>

namespace amanuensis::calamus {

void AppendKey(std::string& output, const std::string& key);
std::string FormatPath(const core::KeyPath& path);

} // namespace amanuensis::calamus
