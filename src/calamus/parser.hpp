#pragma once

#include "amanuensis/calamus/reader.hpp"

#include <string_view>

namespace amanuensis::calamus {

ReadResult ParseDocument(std::string_view input);

} // namespace amanuensis::calamus
