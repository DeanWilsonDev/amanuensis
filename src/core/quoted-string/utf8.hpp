#pragma once

#include <cstdint>
#include <string>

namespace amanuensis::core {

void AppendUtf8(std::string& output, std::uint32_t codePoint);

} // namespace amanuensis::core
