#pragma once

#include "amanuensis/calamus/reader.hpp"

#include <string_view>

namespace amanuensis::calamus {

// Reads Calamus text into a Document: the header, the body as written, and
// the fields whose marks the reader honours. See docs/calamus-spec.md.
ReadResult ParseDocument(std::string_view input);

} // namespace amanuensis::calamus
